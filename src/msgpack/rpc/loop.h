//
// msgpack::rpc::loop - MessagePack-RPC for C++
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
#ifndef MSGPACK_RPC_LOOP_H__
#define MSGPACK_RPC_LOOP_H__

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace msgpack {
namespace rpc {


class loop_impl {
public:
    loop_impl();
    ~loop_impl();
    boost::asio::io_service& io_service();

    void start(size_t num);
    void run(size_t num);
    bool is_running();

    void run_once();
    void flush();
    void join();
    void end();
    void submit(std::function<void ()> callback);

private:
    void add_worker(size_t num);

private:
    boost::asio::io_service m_io_service;
    std::vector< std::shared_ptr<boost::thread> > m_workers;
};


class loop : public std::shared_ptr<loop_impl> {
public:
    loop();
    ~loop();
};


}  // namespace rpc
}  // namespace msgpack

#endif /* msgpack/rpc/loop.h */
