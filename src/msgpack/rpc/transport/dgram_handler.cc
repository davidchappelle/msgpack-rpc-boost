#include "dgram_handler.h"

#include <boost/log/trivial.hpp>
#include <functional>

namespace msgpack {
namespace rpc {

using namespace boost::asio::ip;

class response_sender : public message_sendable {
public:
    response_sender(std::shared_ptr<dgram_handler> handler,
        udp::endpoint& remote) : m_handler(handler), m_remote(remote) { }
    ~response_sender() { };

    void send_data(sbuffer* sbuf) {
        m_handler->send_data(m_remote, sbuf);
    }
    void send_data(std::unique_ptr<vreflife> vbuf) {
        m_handler->send_data(m_remote, std::move(vbuf));
    }

private:
    std::shared_ptr<dgram_handler> m_handler;
    udp::endpoint m_remote;

private:
    response_sender();
    response_sender(const response_sender&);
};

// datagram Hanlder

dgram_handler::dgram_handler(loop lo) :
    m_pac(MSGPACK_UNPACKER_INIT_BUFFER_SIZE),
    m_socket(lo->io_service()),
    m_strand(lo->io_service())
{
}

dgram_handler::~dgram_handler() { }

void dgram_handler::start()
{
    m_socket.async_receive_from(
        boost::asio::buffer(m_pac.buffer(), m_pac.buffer_capacity()), m_remote,
        m_strand.wrap(std::bind(&dgram_handler::on_read, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2)));
}

std::shared_ptr<message_sendable>
dgram_handler::get_response_sender(udp::endpoint& ep)
{
    return std::shared_ptr<message_sendable>(
            new response_sender(shared_from_this(), ep));
}

void dgram_handler::on_read(const boost::system::error_code& err, size_t nbytes)
{
    bool failed = false;
    if (!err) {
        BOOST_LOG_TRIVIAL(debug) << "received from: " << m_remote;
        try {
            m_pac.buffer_consumed(nbytes);
            msgpack::unpacked result;
            while (m_pac.next(&result)) {
                msgpack::object msg = result.get();
                auto_zone z(result.zone().release());
                on_message(msg, std::move(z), m_remote);
            }

            m_pac.reserve_buffer(MSGPACK_UNPACKER_RESERVE_SIZE);
            m_socket.async_receive_from(
                boost::asio::buffer(m_pac.buffer(), m_pac.buffer_capacity()),
                m_remote,
                m_strand.wrap(std::bind(&dgram_handler::on_read,
                    shared_from_this(),
                    std::placeholders::_1, std::placeholders::_2)));
            return;
        }
        catch(std::exception& e)
        {
            BOOST_LOG_TRIVIAL(error) << "on_read() exception: " << boost::diagnostic_information(e).c_str();
            failed = true;
        }
    }
    else if (err.value() != 2) {
        BOOST_LOG_TRIVIAL(error) << "on_read() failed : " << err.value() << ", " <<  err.message();
    }

    if (err || failed) {
        // set exception for orphaned promises
        m_pac.remove_nonparsed_buffer();
    }
}

void dgram_handler::send_data(sbuffer* sbuf)
{
    if (m_socket.is_open() == false)
        return;
    udp::endpoint remote_ep = m_socket.remote_endpoint();
    send_data(remote_ep, sbuf);
}

void dgram_handler::send_data(udp::endpoint& ep, sbuffer* sbuf)
{
    boost::mutex::scoped_lock lock(mutex);
    BOOST_LOG_TRIVIAL(debug) << "send sbuf to : " << ep;
    m_socket.send_to(boost::asio::buffer(sbuf->data(), sbuf->size()), ep);
}

void dgram_handler::send_data(auto_vreflife vbuf)
{
    if (m_socket.is_open() == false)
        return;
    udp::endpoint remote_ep = m_socket.remote_endpoint();
    send_data(remote_ep, std::move(vbuf));
}

void dgram_handler::send_data(udp::endpoint& ep, auto_vreflife vbuf)
{
    boost::mutex::scoped_lock lock(mutex);

    std::vector<boost::asio::const_buffer> buffers;
    const struct iovec *vec = vbuf->vector();
    for (int i = 0; i < (int)vbuf->vector_size(); ++i) {
        buffers.push_back(boost::asio::buffer(vec[i].iov_base, vec[i].iov_len));
    }
    BOOST_LOG_TRIVIAL(debug) << "send vbuf to : " << ep;
    m_socket.send_to(buffers, ep);
}

void dgram_handler::on_message(object msg, auto_zone z, udp::endpoint& ep)
{
    msg_rpc rpc;
    msg.convert(&rpc);

    switch (rpc.type) {
    case REQUEST: {
        msg_request<object, object> req;
        msg.convert(&req);
        on_request(req.msgid, req.method, req.param, std::move(z), ep);
    }
    break;

    case RESPONSE: {
        msg_response<object, object> res;
        msg.convert(&res);
        on_response(res.msgid, res.result, res.error, std::move(z));
    }
    break;

    case NOTIFY: {
        msg_notify<object, object> req;
        msg.convert(&req);
        on_notify(req.method, req.param, std::move(z));
    }
    break;

    default:
        throw msgpack::type_error();
    }
}


}  // namespace rpc
}  // namespace msgpack
