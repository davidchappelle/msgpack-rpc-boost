//
// msgpack::rpc::future - MessagePack-RPC for C++
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
#ifndef MSGPACK_RPC_FUTURE_IMPL_H__
#define MSGPACK_RPC_FUTURE_IMPL_H__

#include "future.h"
#include "session_impl.h"

#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <memory>

namespace msgpack {
namespace rpc {


class future_impl : public std::enable_shared_from_this<future_impl> {
public:
    future_impl(msgid_t msgid, shared_session s, loop lo);
    ~future_impl();

    bool is_ready() const;
    void join();
    void wait();
    bool timed_wait(unsigned ms);
    void recv();

    msgid_t msgid() const
    {
        return m_msgid;
    }

    const object& result() const
    {
        return m_result;
    }

    const object& error() const
    {
        return m_error;
    }

    auto_zone& zone() { return m_zone; }

    void attach_callback(callback_t func);

    void set_result(object result, object error, auto_zone z);

    bool step_timeout();

private:
    msgid_t m_msgid;
    shared_session m_session;
    loop m_loop;

    unsigned int m_timeout;
    callback_t m_callback;

    object m_result;
    object m_error;
    auto_zone m_zone;

    boost::mutex m_mutex;
    boost::condition_variable m_cond;

private:
    future_impl();
    future_impl(const future_impl&);
};


}  // namespace rpc
}  // namespace msgpack

#endif // MSGPACK_RPC_FUTURE_IMPL_H__
