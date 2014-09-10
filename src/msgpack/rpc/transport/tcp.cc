//
// msgpack::rpc::transport::tcp - MessagePack-RPC for C++
//
// Copyright (C) 2009-2010 FURUHASHI Sadayuki
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#include "tcp.h"

#include "stream_handler.h"
#include "../exception.h"
#include "../protocol.h"
#include "../server_impl.h"
#include "../session_impl.h"
#include "../transport_impl.h"
#include "../types.h"

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/log/trivial.hpp>
#include <functional>
#include <vector>

namespace msgpack {
namespace rpc {
namespace transport {
namespace tcp {

class client_transport;
class server_transport;

// TCP Client

class client_socket : public stream_handler
{
public:
    client_socket(client_transport* tran, session_impl* s);
    virtual ~client_socket();

    void on_request(msgid_t msgid, object method, object params, auto_zone z);
    void on_response(msgid_t msgid, object result, object error, auto_zone z);
    void on_notify(object method, object params, auto_zone z);
    void on_system_error(const boost::system::error_code& err);

private:
    int m_connecting;
    client_transport* m_tran;
    weak_session m_session;

    friend class client_transport;
private:
    client_socket();
    client_socket(const client_socket&);
};


class client_transport : public rpc::client_transport {
public:
    client_transport(session_impl* s, const address& addr, const tcp_builder& b);
    ~client_transport();

public:
    // message_sendable
    void send_data(sbuffer* sbuf);
    void send_data(auto_vreflife vbuf);

private:
    session_impl* m_session;

    double m_connect_timeout;
    int m_reconnect_limit;

    std::shared_ptr<client_socket> m_conn;
    // keep io_service running in case of without run()
    // test with test/callback.cc
    boost::asio::io_service::work m_work;
    boost::asio::deadline_timer m_timer;
    boost::mutex mutex;

private:
    void try_connect();
    void on_connect(const boost::system::error_code& err);
    void on_connect_success();
    void on_connect_failed(const boost::system::error_code& err);
    void on_timeout();
    // try connect and throw exception when failed
    void connect();

private:
    client_transport();
    client_transport(const client_transport&);
};


client_socket::client_socket(client_transport* tran, session_impl* s) :
    stream_handler(s->get_loop()),
    m_connecting(0),
    m_tran(tran), m_session(s->shared_from_this())
{ }

client_socket::~client_socket()
{
    shared_session s = m_session.lock();
    if (!s) {
        return;
    }
}

void client_socket::on_request(msgid_t msgid,
        object result, object error, auto_zone z)
{
    throw msgpack::type_error();
}

void client_socket::on_response(msgid_t msgid,
        object result, object error, auto_zone z)
{
    shared_session s = m_session.lock();
    if (!s) {
        throw closed_exception();
    }
    s->on_response(msgid, result, error, std::move(z));
}

void client_socket::on_notify(
        object method, object params, auto_zone z)
{
    shared_session s = m_session.lock();
    if (!s) {
        throw closed_exception();
    }
    s->on_notify(method, params, std::move(z));
}

void client_socket::on_system_error(const boost::system::error_code& err)
{
    shared_session s = m_session.lock();
    if (s) {
        s->on_system_error(err);
    }
    if (socket().is_open())
    try {
        socket().close();
    } catch (...) {
        // ignore
    }
}

// transport

client_transport::client_transport(session_impl* s,
        const address& addr, const tcp_builder& b) :
    m_session(s),
    m_connect_timeout(b.connect_timeout()),
    m_reconnect_limit(b.reconnect_limit()),
    m_conn(new transport::tcp::client_socket(this, m_session)),
    m_work(s->get_loop()->io_service()),
    m_timer(s->get_loop()->io_service())
{
    assert(false == m_conn->socket().is_open());
}

client_transport::~client_transport()
{
    m_conn.reset();
}

inline void client_transport::on_connect_success()
{
    BOOST_LOG_TRIVIAL(debug) << "connect success to " << m_session->get_address();
    m_timer.cancel();
    m_conn->socket().set_option(boost::asio::ip::tcp::no_delay(true));
    m_conn->start();
}

void client_transport::on_connect_failed(const boost::system::error_code& err)
{
    if (err.value() != ETIMEDOUT && m_conn->m_connecting < m_reconnect_limit)
    {
        BOOST_LOG_TRIVIAL(warning) << "connect failed, retrying : " << m_conn->m_connecting;
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        try_connect();
        return;
    }

    BOOST_LOG_TRIVIAL(warning) << "connect to " << m_session->get_address() << " failed.";
    m_timer.cancel();
    m_conn->socket().close();
    m_session->on_connect_failed();
}

void client_transport::on_connect(const boost::system::error_code& err)
{
    if (err) {
        on_connect_failed(err);
    }
    else {
        on_connect_success();
    }
}

void client_transport::try_connect()
{
    if (!m_session->get_loop()->is_running()) {
        m_session->get_loop()->flush();
    }

    address addr = m_session->get_address();
    boost::asio::ip::tcp::endpoint ep(addr.get_addr(), addr.get_port());
    BOOST_LOG_TRIVIAL(debug) << "connecting to " << addr;

    boost::system::error_code ec;
    ++m_conn->m_connecting;
    m_conn->socket().connect(ep, ec);
    on_connect(ec);
}

void client_transport::on_timeout()
{
    if (m_conn->socket().is_open())
        return;

    if (m_timer.expires_at() <= boost::asio::deadline_timer::traits_type::now())
    {
        boost::system::error_code ec;
        ec.assign(ETIMEDOUT, ec.category());
        on_connect_failed(ec);
    }
}

void client_transport::connect()
{
    assert(m_conn->socket().is_open() == false);

    // connection time out
    m_timer.expires_from_now(boost::posix_time::seconds((long)m_connect_timeout));
    m_timer.async_wait(std::bind(&client_transport::on_timeout, this));

    m_conn->m_connecting = 0;
    try_connect();

    if (!m_conn->socket().is_open()) {
        throw connect_error();
    }
}

void client_transport::send_data(sbuffer* sbuf)
{
    if (!m_session->get_loop()->is_running())
        m_session->get_loop()->flush();

    {
        boost::mutex::scoped_lock lock(mutex);
        if (!m_conn->socket().is_open())
            connect();
    }

    m_conn->send_data(sbuf);
}

void client_transport::send_data(auto_vreflife vbuf)
{
    if (!m_session->get_loop()->is_running())
        m_session->get_loop()->flush();

    {
        boost::mutex::scoped_lock lock(mutex);
        if (!m_conn->socket().is_open())
            connect();
    }

    m_conn->send_data(std::move(vbuf));
}

// SERVER

class server_socket : public stream_handler
{
public:
    server_socket(server_transport* tran, shared_server svr);
    virtual ~server_socket();

    void on_request(msgid_t msgid, object method, object params, auto_zone z);
    void on_response(msgid_t msgid, object method, object params, auto_zone z);
    void on_notify(object method, object params, auto_zone z);
    void on_system_error(const boost::system::error_code& err);

private:
    weak_server m_svr;
    server_transport* m_tran;
};


class server_transport : public rpc::server_transport {
public:
    server_transport(server_impl* svr, const address& addr);
    ~server_transport();

    void close();
    void start_accept();
    void on_accept(const boost::system::error_code& error);
    void on_system_error(std::shared_ptr<server_socket> conn);

    virtual int get_connection_num();

private:
    weak_server m_wsvr;
    // acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor m_acceptor;
    std::shared_ptr<server_socket> m_conn;
    // the managed connections
    std::set<std::shared_ptr<server_socket> > m_connections;
    boost::mutex m_mutex;

private:
    server_transport();
    server_transport(const server_transport&);
};


// TCP Server

server_socket::server_socket(server_transport* tran, shared_server svr) :
    stream_handler(svr->get_loop()),
    m_svr(svr),
    m_tran(tran)
{
}

server_socket::~server_socket() { }

void server_socket::on_request(msgid_t msgid,
        object method, object params, auto_zone z)
{
    shared_server svr = m_svr.lock();
    if (!svr) {
        throw closed_exception();
    }
    svr->on_request(get_response_sender(), msgid, method, params, std::move(z));
}

void server_socket::on_response(msgid_t msgid,
        object result, object error, auto_zone z)
{
    throw msgpack::type_error();
}

void server_socket::on_notify(
        object method, object params, auto_zone z)
{
    shared_server svr = m_svr.lock();
    if (!svr) {
        throw closed_exception();
    }
    svr->on_notify(method, params, std::move(z));
}

void server_socket::on_system_error(const boost::system::error_code& err)
{
    m_tran->on_system_error(
        std::static_pointer_cast<server_socket>(shared_from_this()));
}


server_transport::server_transport(server_impl* svr, const address& addr) :
    m_acceptor(svr->get_loop()->io_service()),
    m_conn()
{
    m_wsvr = weak_server(
        std::static_pointer_cast<server_impl>(svr->shared_from_this()));

    // open the acceptor with option to reuse the address
    boost::asio::ip::tcp::endpoint ep(addr.get_addr(), addr.get_port());

    m_acceptor.open(ep.protocol());
    m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    m_acceptor.bind(ep);
    m_acceptor.listen();

    start_accept();
}

server_transport::~server_transport()
{
    close();
}

void server_transport::close()
{
    m_acceptor.close();
    m_conn.reset();
    // stop all connections
    boost::mutex::scoped_lock lock(m_mutex);
    std::for_each(m_connections.begin(), m_connections.end(),
            std::bind(&server_socket::stop, std::placeholders::_1));
    m_connections.clear();
}

void server_transport::start_accept()
{
    m_conn.reset(new server_socket(this, m_wsvr.lock()));
    m_acceptor.async_accept(m_conn->socket(),
        std::bind(&server_transport::on_accept, this, std::placeholders::_1));
}

void server_transport::on_system_error(std::shared_ptr<server_socket> conn)
{
    boost::mutex::scoped_lock lock(m_mutex);
    m_connections.erase(conn);
    conn->stop();
}

void server_transport::on_accept(const boost::system::error_code& err)
{
    if (!err) {
        boost::mutex::scoped_lock lock(m_mutex);
        m_connections.insert(m_conn);
        BOOST_LOG_TRIVIAL(debug) << "server_socket accepted : " << m_conn->socket().remote_endpoint();
        m_conn->socket().set_option(boost::asio::ip::tcp::no_delay(true));
        m_conn->start();
    }

    start_accept();
}

int server_transport::get_connection_num()
{
    return m_connections.size();
}

}  // namespace tcp
}  // namespace transport


tcp_builder::tcp_builder() :
    m_connect_timeout(10.0),
    m_reconnect_limit(3)
{ }

tcp_builder::~tcp_builder() { }

std::unique_ptr<client_transport>
tcp_builder::build(session_impl* s, const address& addr) const
{
    return std::unique_ptr<client_transport>(
            new transport::tcp::client_transport(s, addr, *this));
}


tcp_listener::tcp_listener(const std::string& host, uint16_t port) :
    m_addr(address(host, port)) { }

tcp_listener::tcp_listener(const address& addr) :
    m_addr(addr) { }

tcp_listener::~tcp_listener() { }

std::unique_ptr<server_transport> tcp_listener::listen(server_impl* svr) const
{
    return std::unique_ptr<server_transport>(
            new transport::tcp::server_transport(svr, m_addr));
}


}  // namespace rpc
}  // namespace msgpack
