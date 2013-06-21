//
// msgpack::rpc::transport::udp - MessagePack-RPC for C++
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
#include "../types.h"
#include <iostream>
#include <glog/logging.h>
#include <vector>

#include "dgram_handler.h"
#include "udp.h"

namespace msgpack {
namespace rpc {
namespace transport {
namespace udp {

class client_transport;
class server_transport;


class client_socket : public dgram_handler {
public:
    client_socket(session_impl* s, bool broadcast);
    ~client_socket();

    void connect(const address& addr);

    void on_request(msgid_t msgid, object method, object params, auto_zone z,
                    boost::asio::ip::udp::endpoint& ep);
    void on_response(msgid_t msgid, object result, object error, auto_zone z);
    void on_notify(object method, object params, auto_zone z);

private:
    weak_session m_session;
    bool m_broadcast;

private:
    client_socket();
    client_socket(const client_socket&);
};


class client_transport : public rpc::client_transport {
public:
    client_transport(session_impl* s, const address& addr, const udp_builder& b);
    ~client_transport();

public:
    void send_data(sbuffer* sbuf);
    void send_data(auto_vreflife vbuf);

private:
    session_impl* m_session;
    boost::shared_ptr<client_socket> m_conn;
    boost::asio::io_service::work m_work;

private:
    client_transport();
    client_transport(const client_transport&);
};


client_socket::client_socket(session_impl* s, bool broadcast) :
    dgram_handler(s->get_loop()),
    m_session(s->shared_from_this()),
    m_broadcast(broadcast)
{
}

client_socket::~client_socket() { }

void client_socket::connect(const address& addr)
{
    boost::asio::ip::udp::endpoint ep(addr.get_addr(), addr.get_port());
    DLOG(INFO) << "connecting to " << addr;

    boost::system::error_code ec;
    socket().connect(ep, ec);
    if (ec) {
        DLOG(WARNING) << "connect failed : " << ec.message();
        socket().close();
        return;
    }
    socket().set_option(boost::asio::ip::udp::socket::reuse_address(true));
    if (m_broadcast)
        socket().set_option(boost::asio::socket_base::broadcast(true));

    start();
}

void client_socket::on_request(msgid_t msgid,
        object result, object error, auto_zone z,
        boost::asio::ip::udp::endpoint& ep)
{
    throw msgpack::type_error();
}

void client_socket::on_response(msgid_t msgid,
            object result, object error, auto_zone z)
{
    shared_session s = m_session.lock();
    if(!s) {
        throw closed_exception();
    }
    s->on_response(msgid, result, error, z);
}

void client_socket::on_notify(
        object method, object params, auto_zone z)
{
    shared_session s = m_session.lock();
    if (!s) {
        throw closed_exception();
    }
    s->on_notify(method, params, z);
}


client_transport::client_transport(session_impl* s,
        const address& addr, const udp_builder& b) :
    m_session(s), 
    m_conn(new transport::udp::client_socket(s, b.get_broadcast())),
    m_work(s->get_loop()->io_service())
{
    m_conn->connect(addr);
}

client_transport::~client_transport()
{
}

void client_transport::send_data(sbuffer* sbuf)
{
    if (!m_session->get_loop()->is_running())
        m_session->get_loop()->flush();

    if (!m_conn->socket().is_open())
        m_conn->connect(m_session->get_address());

    m_conn->send_data(sbuf);
}

void client_transport::send_data(auto_vreflife vbuf)
{
    if (!m_session->get_loop()->is_running())
        m_session->get_loop()->flush();

    if (!m_conn->socket().is_open())
        m_conn->connect(m_session->get_address());

    m_conn->send_data(vbuf);
}

// SERVER

class server_socket : public dgram_handler
{
public:
    server_socket(shared_server svr);
    ~server_socket();

    void listen(const address& addr);

    // dgram_handler interface
    void on_request(msgid_t msgid, object method, object params, auto_zone z,
                    boost::asio::ip::udp::endpoint& ep);
    void on_response(msgid_t msgid, object method, object params, auto_zone z);
    void on_notify(object method, object params, auto_zone z);

private:
    weak_server m_svr;

private:
    server_socket();
    server_socket(const server_socket&);
};


class server_transport : public rpc::server_transport {
public:
    server_transport(server_impl* svr, const address& addr);
    ~server_transport();

    void close();
    virtual int get_connection_num();

private:
    weak_server m_wsvr;
    boost::shared_ptr<server_socket> m_conn;

private:
    server_transport();
    server_transport(const server_transport&);
};


server_socket::server_socket(shared_server svr) :
    dgram_handler(svr->get_loop()),
    m_svr(svr) { }

server_socket::~server_socket() { }

void server_socket::listen(const address& addr)
{
    boost::asio::ip::udp::endpoint ep(addr.get_addr(), addr.get_port());

    socket().open(boost::asio::ip::udp::v4());
    socket().set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket().bind(ep);
    start();
}

void server_socket::on_request(
        msgid_t msgid, object method, object params, auto_zone z,
        boost::asio::ip::udp::endpoint& ep)
{
    shared_server svr = m_svr.lock();
    if (!svr) {
        throw closed_exception();
    }
    svr->on_request(get_response_sender(ep), msgid, method, params, z);
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
    if(!svr) {
        throw closed_exception();
    }
    svr->on_notify(method, params, z);
}


server_transport::server_transport(server_impl* svr, const address& addr)
{
    m_wsvr = weak_server(
        boost::static_pointer_cast<server_impl>(svr->shared_from_this()));

    m_conn.reset(new transport::udp::server_socket(m_wsvr.lock()));
    m_conn->listen(addr);
}

server_transport::~server_transport()
{
    close();
}

void server_transport::close()
{
}

int server_transport::get_connection_num()
{
    throw std::runtime_error("not supported in udp");
}

}  // namespace udp
}  // namespace transport


udp_builder::udp_builder() : m_broadcast(false) { }

udp_builder::~udp_builder() { }

void udp_builder::set_broadcast(bool val) {
    m_broadcast = val;
}

bool udp_builder::get_broadcast() const {
    return m_broadcast;
}

std::auto_ptr<client_transport> udp_builder::build(session_impl* s, const address& addr) const
{
    return std::auto_ptr<client_transport>(new transport::udp::client_transport(s, addr, *this));
}


udp_listener::udp_listener(const std::string& host, uint16_t port) :
    m_addr(address(host, port)) { }

udp_listener::udp_listener(const address& addr) :
    m_addr(addr) { }

udp_listener::~udp_listener() { }

std::auto_ptr<server_transport> udp_listener::listen(server_impl* svr) const
{
    return std::auto_ptr<server_transport>(
            new transport::udp::server_transport(svr, m_addr));
}


}  // namespace rpc
}  // namespace msgpack

