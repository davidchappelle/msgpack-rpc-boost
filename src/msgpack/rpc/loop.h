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
#include <boost/asio/deadline_timer.hpp>
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

    int add_signal(int signo, std::function<void
             (const boost::system::error_code& error, int signo)> handler);
    void remove_signal(int id);

    void add_timer(int sec, std::function<bool ()> handler);
    void step_timeout(int sec, std::function<bool ()> handler,
             const boost::system::error_code& err);

private:
    void add_worker(size_t num);

private:
    boost::asio::io_service m_io;
#if BOOST_VERSION >= 104800
    boost::asio::signal_set m_sigset;
#endif
    boost::asio::deadline_timer m_timer;
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
