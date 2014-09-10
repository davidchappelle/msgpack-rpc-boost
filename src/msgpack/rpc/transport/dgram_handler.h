#ifndef MSGPACK_RPC_TRANSPORT_DGRAM_HANDLER_H__
#define MSGPACK_RPC_TRANSPORT_DGRAM_HANDLER_H__

#include "../protocol.h"
#include "../server_impl.h"
#include "../session_impl.h"
#include "../transport_impl.h"
#include "../types.h"

#include <boost/asio.hpp>
#include <memory>

namespace msgpack {
namespace rpc {

class closed_exception : public std::exception { };

class dgram_handler :  public message_sendable,
    public std::enable_shared_from_this<dgram_handler>
{
public:
    dgram_handler(loop lo);
    ~dgram_handler();

    boost::asio::ip::udp::socket& socket() { return m_socket; }

    void start();
    void on_read(const boost::system::error_code& err, size_t nbytes);

    std::shared_ptr<message_sendable>
        get_response_sender(boost::asio::ip::udp::endpoint& ep);

    // message_sendable
    void send_data(sbuffer* sbuf);
    void send_data(auto_vreflife vbuf);

    void send_data(boost::asio::ip::udp::endpoint& ep, sbuffer* sbuf);
    void send_data(boost::asio::ip::udp::endpoint& ep, auto_vreflife vbuf);

    // process message
    void on_message(object msg, auto_zone z, boost::asio::ip::udp::endpoint& ep);
    virtual void on_request(msgid_t msgid, object method, object params, auto_zone z,
                            boost::asio::ip::udp::endpoint& ep) = 0;
    virtual void on_response(msgid_t msgid, object result, object error, auto_zone z) = 0;
    virtual void on_notify(object method, object params, auto_zone z) = 0;

protected:
    unpacker m_pac;
    boost::asio::ip::udp::socket m_socket;
    boost::asio::io_service::strand m_strand;
    boost::asio::ip::udp::endpoint m_remote;
    boost::mutex mutex;
};


}  // namespace rpc
}  // namespace msgpack

#endif /* transport/dgram_handler.h */
