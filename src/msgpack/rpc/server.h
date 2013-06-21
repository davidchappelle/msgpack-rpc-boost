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
#ifndef MSGPACK_RPC_SERVER_H__
#define MSGPACK_RPC_SERVER_H__

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "session_pool.h"
#include "request.h"

namespace msgpack {
namespace rpc {


class dispatcher : public boost::enable_shared_from_this<dispatcher> {
public:
    virtual void dispatch(request req) = 0;
};


class server : public session_pool {
public:
    server(loop lo = loop());
    server(const builder& b, loop lo = loop());
    virtual ~server();

    void serve(boost::shared_ptr<dispatcher> dp);
    void close();

    void listen(const listener& l);
    void listen(const address& addr);
    void listen(const std::string& host, uint16_t port);

    int get_connection_num();
    int get_request_num();

    class base;

private:
    server(const server&);
};


class server::base : public dispatcher {
public:
    base(loop lo = loop()) : m_instance(lo) { }
    base(const builder& b, loop lo = loop()) : m_instance(b, lo) { }
    ~base() { }

    server& listen(const listener& l) {
        m_instance.serve(shared_from_this());
        m_instance.listen(l);
        return m_instance;
    }

    server& listen(const address& addr) {
        m_instance.serve(shared_from_this());
        m_instance.listen(addr);
        return m_instance;
    }

    server& listen(const std::string& host, uint16_t port) {
        m_instance.listen(host, port);
        return m_instance;
    }

private:
    rpc::server m_instance;
};


}  // namespace rpc
}  // namespace msgpack

#endif /* msgpack/rpc/server.h */
