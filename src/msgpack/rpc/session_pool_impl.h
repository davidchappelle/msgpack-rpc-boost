//
// msgpack::rpc::session_pool - MessagePack-RPC for C++
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
#ifndef MSGPACK_RPC_SESSION_POOL_IMPL_H__
#define MSGPACK_RPC_SESSION_POOL_IMPL_H__

#include "session_pool.h"
#include "transport_impl.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

namespace msgpack {
namespace rpc {


class session_pool_impl {
public:
    session_pool_impl(const builder& b, loop lo);
    ~session_pool_impl();

    session get_session(const address& addr);

    session get_session(const std::string& host, uint16_t port)
        { return get_session(address(host, port)); }

    loop get_loop()
        { return m_loop; }

public:
    void step_timeout();
    void set_timeout(unsigned int sec);

private:
    struct entry_t {
        shared_session session;
        unsigned int ttl;
        entry_t(const shared_session &s, unsigned int c) :
            session(s), ttl(c)
        { }
    };

    loop m_loop;
    std::map<address, entry_t> m_table;
    boost::mutex m_table_mutex;
    std::auto_ptr<builder> m_builder;

private:
    session_pool_impl(const session_pool_impl&);
};


}  // namespace rpc
}  // namespace msgpack

#endif /* msgpack/rpc/session_pool_impl.h */
