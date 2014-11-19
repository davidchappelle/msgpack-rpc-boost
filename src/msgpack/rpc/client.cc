//
// msgpack::rpc::client - MessagePack-RPC for C++
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
#include "client.h"
#include "session_impl.h"
#include "transport/tcp.h"

#include <functional>

namespace msgpack {
namespace rpc {


client::client(const std::string& host, uint16_t port, loop lo) :
    session(session_impl::create(tcp_builder(), address(host, port), lo))
{
}

client::client(const std::string& addr, loop lo) :
    session(session_impl::create(tcp_builder(), address(addr), lo))
{
}

client::client(const address& addr, loop lo) :
    session(session_impl::create(tcp_builder(), addr, lo))
{
}

client::client(const builder& b, const address& addr, loop lo) :
    session(session_impl::create(b, addr, lo))
{
}

client::~client() { }

}  // namespace rpc
}  // namespace msgpack

