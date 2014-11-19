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
#ifndef MSGPACK_RPC_SESSION_POOL_H__
#define MSGPACK_RPC_SESSION_POOL_H__

#include "session.h"
#include "address.h"
#include "transport.h"
#include "impl_fwd.h"
#include "types.h"
#include <string>

namespace msgpack {
namespace rpc {


class session_pool
{
public:
    session_pool(loop lo = loop());
    session_pool(const builder& b, loop lo = loop());
    virtual ~session_pool();

    session get_session(const address& addr);
    session get_session(const std::string& host, uint16_t port);
    session get_session(const std::string& address);

    loop get_loop();

    // loop util functions
    void start(size_t num);
    void run(size_t num);
    void run_once();
    void end();
    void join();
    bool is_running();
    void set_timeout(unsigned int sec);

protected:
    session_pool(shared_session_pool pimpl);
    shared_session_pool m_pimpl;

private:
    session_pool(const session_pool&);
};


}  // namespace rpc
}  // namespace msgpack

#endif /* msgpack/rpc/session_pool.h */
