#ifndef MSGPACK_RPC_TRANSPORT_STREAM_HANDLER_H__
#define MSGPACK_RPC_TRANSPORT_STREAM_HANDLER_H__

#include "../types.h"
#include "../protocol.h"
#include "../session_impl.h"
#include "../server_impl.h"
#include "../transport_impl.h"

namespace msgpack {
namespace rpc {

class closed_exception : public std::exception { };


class stream_handler :  public message_sendable,
    public boost::enable_shared_from_this<stream_handler>
{
public:
    stream_handler(loop lo);
    virtual ~stream_handler();

    boost::asio::ip::tcp::socket& socket() { return m_socket; }
    boost::shared_ptr<message_sendable> get_response_sender() {
        return boost::static_pointer_cast<message_sendable>(shared_from_this());
    }

    void start();
    void stop();
    void on_read(const boost::system::error_code& err, size_t bytes_transferred);

    // message_sendable
    void send_data(sbuffer* sbuf);
    void send_data(auto_vreflife vbuf);

    // process message
    void on_message(object msg, auto_zone z);
    virtual void on_request(msgid_t msgid, object method, object params, auto_zone z) = 0;
    virtual void on_response(msgid_t msgid, object result, object error, auto_zone z) = 0;
    virtual void on_notify(object method, object params, auto_zone z) = 0;

    // error handler
    virtual void on_system_error(const boost::system::error_code& err) = 0;

protected:
    boost::scoped_ptr<unpacker> m_pac;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::io_service::strand m_strand;
    boost::mutex mutex;
};


}  // namespace rpc
}  // namespace msgpack

#endif /* transport/stream_handler.h */
