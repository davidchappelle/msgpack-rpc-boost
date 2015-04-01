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
#include "exception_impl.h"
#include "future_impl.h"

#include <msgpack.hpp>
#include <boost/log/trivial.hpp>

namespace msgpack {
namespace rpc {


future_impl::future_impl(msgid_t msgid, shared_session s, loop lo) :
    m_msgid(msgid),
    m_session(s),
    m_loop(lo),
    m_timeout(s->get_timeout())
{
}

future_impl::~future_impl()
{
}

bool future_impl::is_ready() const
{
    return !m_session;
}

void future_impl::wait()
{
    boost::mutex::scoped_lock lk(m_mutex);
    while (m_session) {
        m_cond.wait(lk);
    }
}

bool future_impl::timed_wait(unsigned ms)
{
    boost::mutex::scoped_lock lk(m_mutex);
    while (m_session) {
        if (!m_cond.timed_wait(lk, boost::posix_time::milliseconds(ms))) {
            lk.unlock();
            set_result(object(), TIMEOUT_ERROR, auto_zone());
            return false;
        }
    }
    return true;
}

void future_impl::recv()
{
    while (m_session) {
        m_loop->run_once();
    }
}

void future_impl::join()
{
    if (m_loop->is_running()) {
        // wait();
        timed_wait(m_timeout * 1000);
    } else {
        recv();
    }
}

static void callback_real(callback_t callback, future f)
{
    try {
        callback(f);
    } catch (std::exception& e) {
        BOOST_LOG_TRIVIAL(warning) << "response callback error: " <<  e.what();
    } catch (...) {
        BOOST_LOG_TRIVIAL(warning) << "response callback error: unknown error";
    }
}

void future_impl::attach_callback(callback_t func)
{
    boost::mutex::scoped_lock lk(m_mutex);

    assert(func);
    if (!m_session) {
        // callback_real(func, future(shared_from_this()));
        m_loop->submit(std::bind(&callback_real, func,
                       future(shared_from_this())));
    } else {
        m_callback = func;
    }
}

void future_impl::set_result(object result, object error, auto_zone z)
{
    boost::mutex::scoped_lock lk(m_mutex);
    m_result = result;
    m_error = error;
    m_zone = std::move(z);
    m_session.reset();

    m_cond.notify_all();

    if (m_callback) {
        callback_real(m_callback, future(shared_from_this()));
        m_callback = nullptr;
    }
}

bool future_impl::step_timeout()
{
    if (m_timeout > 0) {
        --m_timeout;
        return false;
    }
    return true;
}

// FUTURE

future::future() : m_pimpl()
{
}

future::future(shared_future pimpl) : m_pimpl(pimpl)
{
}

future::future(std::string& method, shared_future pimpl) :
    m_method(method), m_pimpl(pimpl)
{
}

future::~future()
{
}

msgid_t future::msgid() const
{
    if (!m_pimpl)
        return 0;
    return m_pimpl->msgid();
}

const std::string& future::method() const
{
    return m_method;
}

object future::get_impl()
{
    if (!m_pimpl) {
        throw std::runtime_error("null future reference");
    }
    if (!m_pimpl->is_ready()) {
        m_pimpl->join();
    }
    if (!m_pimpl->error().is_nil()) {
        throw_exception(this);
    }
    return m_pimpl->result();
}

bool future::is_nil() const
{
    return m_pimpl == NULL;
}

bool future::is_ready() const
{
    return m_pimpl->is_ready();
}

void future::wait(bool rethrow)
{
    if (m_pimpl && !m_pimpl->is_ready())
        m_pimpl->join();

    if (rethrow) {
        get_impl();
    }
}

bool future::timed_wait(unsigned ms)
{
    return m_pimpl->timed_wait(ms);
}

object future::result() const
{
    return m_pimpl->result();
}

object future::error() const
{
    return m_pimpl->error();
}

future& future::attach_callback(std::function<void (future)> func)
{
    m_pimpl->attach_callback(func);
    return *this;
}

auto_zone& future::zone()
{
    return m_pimpl->zone();
}

const auto_zone& future::zone() const
{
    return m_pimpl->zone();
}

bool future::operator< (const future& f) const
{
    return m_pimpl < f.m_pimpl;
}

bool future::operator== (const future& f) const
{
    return m_pimpl == f.m_pimpl;
}


}  // namespace rpc
}  // namespace msgpack
