#include "stream_handler.h"

#include "../types.h"
#include "../protocol.h"
#include "../session_impl.h"
#include "../server_impl.h"
#include "../transport_impl.h"
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

namespace msgpack {
namespace rpc {


#ifndef MSGPACK_RPC_STREAM_BUFFER_SIZE
#define MSGPACK_RPC_STREAM_BUFFER_SIZE (256*1024)
#endif

#ifndef MSGPACK_RPC_STREAM_RESERVE_SIZE
#define MSGPACK_RPC_STREAM_RESERVE_SIZE (32*1024)
#endif


stream_handler::stream_handler(loop lo) :
    m_socket(lo->io_service()),
    m_strand(lo->io_service())
{
    m_pac.reset(new unpacker(MSGPACK_RPC_STREAM_BUFFER_SIZE));
}

stream_handler::~stream_handler() { }

void stream_handler::start()
{
    m_pac->reserve_buffer(MSGPACK_RPC_STREAM_RESERVE_SIZE);
    m_socket.async_read_some(
        boost::asio::buffer(m_pac->buffer(), m_pac->buffer_capacity()),
        m_strand.wrap(boost::bind(&stream_handler::on_read, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)));
}

void stream_handler::stop()
{
    if (!m_socket.is_open())
        return;
    try {
        DLOG(INFO) << "stream_handler::stop : " << m_socket.remote_endpoint();
        m_socket.close();
    }
    catch(boost::system::system_error& e) {
        boost::system::error_code ec = e.code();
        if (ec.value() == ENOTCONN)
            return;
        DLOG(ERROR) << "stream_handler::stop : " << ec.value() << ", "
            <<  ec.message();
    }
}

void stream_handler::on_read(const boost::system::error_code& err, size_t nbytes)
{
    bool failed = false;
    if (!err) {
        try {
            m_pac->buffer_consumed(nbytes);
            msgpack::unpacked result;
            while (m_pac->next(&result)) {
                msgpack::object msg = result.get();
                // std::unique_ptr<msgpack::zone> z(m_pac->release_zone());
                std::unique_ptr<msgpack::zone> z(result.zone().release());
                DLOG(INFO) << "obj received: " << msg;
                on_message(msg, std::move(z));
            }
            if (m_pac->message_size() > 10 * 1024 * 1024) {
                throw std::runtime_error("message is too large");
            }

            start();
            return;
        }
        catch(std::exception& e)
        {
            LOG(ERROR) << "on_read() exception: "
                << boost::diagnostic_information(e).c_str();
            failed = true;
        }
    }
    else {
        LOG_IF(ERROR, err.value() != 2) << "on_read() failed : "
            << err.value() << ", " <<  err.message();
    }

    if (err || failed) {
        // set exception for orphaned promises
        on_system_error(err);
        m_pac->remove_nonparsed_buffer();
    }
}

void stream_handler::send_data(sbuffer* sbuf)
{
    if (!m_socket.is_open())
        return;

    try {
        boost::mutex::scoped_lock lock(mutex);
        boost::asio::write(m_socket,
            boost::asio::buffer(sbuf->data(), sbuf->size()));
    } catch (boost::system::system_error& e) {
        boost::system::error_code ec = e.code();
        on_system_error(ec);
        LOG(ERROR) << "send_data() failed : " << ec.value() << ", "
            <<  ec.message();
    }
}

void stream_handler::send_data(auto_vreflife vbuf)
{
    if (!m_socket.is_open())
        return;

    std::vector<boost::asio::const_buffer> buffers;
    const struct iovec *vec = vbuf->vector();
    int veclen = (int)vbuf->vector_size();

    for (int i = 0; i < veclen; ++i) {
        buffers.push_back(boost::asio::buffer(vec[i].iov_base, vec[i].iov_len));
    }

    try {
        boost::mutex::scoped_lock lock(mutex);
        boost::asio::write(m_socket, buffers);
    } catch (boost::system::system_error& e) {
        boost::system::error_code ec = e.code();
        on_system_error(ec);
        LOG(ERROR) << "send_data() failed : " << ec.value() << ", "
            <<  ec.message();
    }
}

void stream_handler::on_message(object msg, auto_zone z)
{
    msg_rpc rpc;
    msg.convert(&rpc);

    switch (rpc.type) {
    case REQUEST: {
        msg_request<object, object> req;
        msg.convert(&req);
        on_request(req.msgid, req.method, req.param, std::move(z));
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
