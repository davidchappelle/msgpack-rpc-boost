//
// msgpack::rpc::server - MessagePack-RPC for C++
//
// Copyright (C) 2010 FURUHASHI Sadayuki
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
#include "server.h"
#include "server_impl.h"
#include "request_impl.h"
#include "transport.h"
#include "transport/tcp.h"

namespace msgpack {
namespace rpc {


server_impl::server_impl(const builder& b, loop lo) :
    session_pool_impl(b, lo),
    m_dp()
{ }

server_impl::~server_impl()
{
    close();
}

void server_impl::serve(boost::shared_ptr<dispatcher> dp)
{
    m_dp = dp;
}

void server_impl::listen(const listener& l)
{
    m_stran = l.listen(this);
}

void server_impl::close()
{
    m_stran.reset();
}

int server_impl::get_connection_num()
{
    if (m_stran.get())
        return m_stran->get_connection_num();
    return 0;
}

int server_impl::get_request_num()
{
    return 0;  // works sync
}

void server_impl::on_request(
        shared_message_sendable ms, msgid_t msgid,
        object method, object params, auto_zone z)
{
    shared_request sr(new request_impl(
            ms, msgid,
            method, params, z));
    m_dp->dispatch(request(sr));
}

void server_impl::on_notify(
        object method, object params, auto_zone z)
{
    shared_request sr(new request_impl(
            shared_message_sendable(), 0,
            method, params, z));
    m_dp->dispatch(request(sr));
}

// SERVER

server::server(loop lo) :
    session_pool(shared_session_pool(new server_impl(tcp_builder(), lo)))
{
}

server::server(const builder& b, loop lo) :
    session_pool(shared_session_pool(new server_impl(b, lo)))
{
}

server::~server()
{
    close();
}

void server::serve(boost::shared_ptr<dispatcher> dp)
{
    static_cast<server_impl*>(m_pimpl.get())->serve(dp);
}

void server::close()
{
    // to prevent trailing handle_accept()
    get_loop()->end();
    static_cast<server_impl*>(m_pimpl.get())->close();
}

void server::listen(const listener& l)
{
    static_cast<server_impl*>(m_pimpl.get())->listen(l);
}

void server::listen(const address& addr)
{
    listen(tcp_listener(addr));
}

void server::listen(const std::string& host, uint16_t port)
{
    listen(address(host, port));
}

int server::get_connection_num()
{
    return static_cast<server_impl*>(m_pimpl.get())->get_connection_num();
}

int server::get_request_num()
{
    return static_cast<server_impl*>(m_pimpl.get())->get_request_num();
}


}  // namespace rpc
}  // namespace msgpack
