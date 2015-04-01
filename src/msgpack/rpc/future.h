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
#ifndef MSGPACK_RPC_FUTURE_H__
#define MSGPACK_RPC_FUTURE_H__

#include "impl_fwd.h"
#include "protocol.h"
#include "types.h"

#include <functional>
#include <msgpack_fwd.hpp>
#include <msgpack/adaptor/nil.hpp>
#include <utility> // for std::move()

namespace msgpack {
namespace rpc {

class future_impl;

class future {
public:
    future();
    future(shared_future pimpl);
    future(std::string& m, shared_future pimpl);
    ~future();

    msgid_t msgid() const;
    const std::string& method() const;

    template<typename T> T get();
    template<typename T> T get(auto_zone* z);
    template<typename T> T result_as() const;
    template<typename T> T error_as() const;

    bool is_nil() const;
    bool is_ready() const;

    /// Wait until the result is ready
    void wait(bool rethrow=true);

    /// Wait until the result is ready, or the time specified by 'ms' has elapsed
    /// @param ms - time in milliseconds
    bool timed_wait(unsigned ms);

    msgpack::object result() const;
    msgpack::object error() const;

    auto_zone& zone();
    const auto_zone& zone() const;

    future& attach_callback(std::function<void (future)> func);

    // for std::map and std::list
    bool operator< (const future& f) const;
    bool operator== (const future& f) const;

    template<typename T> class type;

private:
    // msgid and method is added to handle error
    std::string m_method;

    shared_future m_pimpl;
    msgpack::object get_impl();
};


template <typename T>
class future::type : public future {
public:
    type(const future& f) : future(f) { }
    ~type() { }

    T get()
        { return future::get<T>(); }

    T get(auto_zone* z)
        { return future::get<T>(z); }

    T result() const
        { return future::result().template as<T>(); }
};

template <>
class future::type<void> : public future {
public:
    type(const future& f) : future(f) { }
    ~type() { }

    void get()
        { future::get<msgpack::type::nil>(); }

    void get(auto_zone* z)
        { future::get<msgpack::type::nil>(z); }

    void result() const
        { future::result().as<msgpack::type::nil>(); }
};


typedef std::function<void (future)> callback_t;


template <typename T>
T future::get()
{
    return get_impl().as<T>();
}

template <> inline
void future::get<void>()
{
    get_impl().as<msgpack::type::nil>();
}

template <typename T>
T future::get(auto_zone* z)
{
    msgpack::object obj = get_impl();
    *z = std::move(zone());
    return obj.as<T>();
}

template <> inline
void future::get<void>(auto_zone* z)
{
    msgpack::object obj = get_impl();
    *z = std::move(zone());
    obj.as<msgpack::type::nil>();
}

template <typename T>
T future::result_as() const
{
    return result().as<T>();
}

template <typename T>
T future::error_as() const
{
    return error().as<T>();
}


}  // namespace rpc
}  // namespace msgpack

#endif //  MSGPACK_RPC_FUTURE_H__
