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
#ifndef MSGPACK_RPC_SERVER_IMPL_H__
#define MSGPACK_RPC_SERVER_IMPL_H__

#include "server.h"
#include "session_pool_impl.h"

namespace msgpack {
namespace rpc {


class server_impl : public session_pool_impl,
    public boost::enable_shared_from_this<server_impl>
{
public:
    server_impl(const builder&, loop lo);
    ~server_impl();

    void serve(boost::shared_ptr<dispatcher> dp);
    void listen(const listener& l);
    void close();

    int get_connection_num();
    int get_request_num();

public:
    void on_request(shared_message_sendable ms, msgid_t msgid,
            object method, object params, auto_zone z);

    void on_notify(object method, object params, auto_zone z);

private:
    boost::shared_ptr<dispatcher> m_dp;
    std::auto_ptr<server_transport> m_stran;

private:
    server_impl(const server_impl&);
};


}  // namespace rpc
}  // namespace msgpack

#endif /* msgpack/rpc/server.h */
