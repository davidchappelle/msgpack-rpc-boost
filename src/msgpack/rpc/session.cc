//
// msgpack::rpc::session - MessagePack-RPC for C++
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
#include "atomic_ops.h"
#include "session.h"
#include "session_impl.h"
#include "future_impl.h"
#include "request_impl.h"
#include "exception_impl.h"
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>


namespace msgpack {
namespace rpc {


session_impl::session_impl(const address& addr, loop lo) :
    m_addr(addr),
    m_loop(lo),
    m_msgid_rr(1),
    m_timeout(30)
{ }

session_impl::~session_impl() { }

void session_impl::build(const builder& b)
{
    m_tran = b.build(this, m_addr);
    m_timeout = b.get_timeout();
}

shared_session
session_impl::create(const builder& b, const address addr, loop lo)
{
    shared_session s(new session_impl(addr, lo));
    s->build(b);
    return s;
}

future session_impl::send_request_impl(msgid_t msgid, std::string method,
    sbuffer* sbuf)
{
    DLOG(INFO) << "sending... msgid=" << msgid;
    shared_future f(new future_impl(msgid, shared_from_this(), m_loop));
    m_reqtable.insert(msgid, f);

    m_tran->send_data(sbuf);
    return future(method, f);
}

future session_impl::send_request_impl(msgid_t msgid, std::string method,
    std::auto_ptr<with_shared_zone<vrefbuffer> > vbuf)
{
    DLOG(INFO) << "sending... msgid=" <<  msgid;
    shared_future f(new future_impl(msgid, shared_from_this(), m_loop));
    m_reqtable.insert(msgid, f);

    m_tran->send_data(vbuf);
    return future(method, f);
}

void session_impl::send_notify_impl(sbuffer* sbuf)
{
    m_tran->send_data(sbuf);
}

void session_impl::send_notify_impl(auto_vreflife vbuf)
{
    m_tran->send_data(vbuf);
}

msgid_t session_impl::next_msgid()
{
    return atomic_increment(&m_msgid_rr);
}

void session_impl::step_timeout()
{
    std::vector<shared_future> timedout;
    m_reqtable.step_timeout(&timedout);
    if (!timedout.empty()) {
        for (std::vector<shared_future>::iterator it = timedout.begin();
                it != timedout.end(); ++it) {
            shared_future& f = *it;
            f->set_result(object(), TIMEOUT_ERROR, auto_zone());
            DLOG(WARNING) << "timeout " << f->msgid();
        }
    }
}

void session_impl::step_timeout(std::vector<shared_future>* timedout)
{
    m_reqtable.step_timeout(timedout);
}

void session_impl::on_connect_failed()
{
    std::vector<shared_future> all;
    m_reqtable.take_all(&all);
    for (std::vector<shared_future>::iterator it = all.begin();
            it != all.end(); ++it) {
        shared_future& f = *it;
        f->set_result(object(), CONNECT_ERROR, auto_zone());
    }
}

void session_impl::on_system_error(const boost::system::error_code& err)
{
    std::vector<shared_future> all;
    m_reqtable.take_all(&all);
    for (std::vector<shared_future>::iterator it = all.begin();
            it != all.end(); ++it) {
        shared_future& f = *it;
        // FIXME
        f->set_result(object(), CONNECT_ERROR, auto_zone());
    }
}

void session_impl::on_response(msgid_t msgid,
                               object result, object error, auto_zone z)
{
    DLOG(INFO) << "response msgid=" << msgid;
    shared_future f = m_reqtable.take(msgid);
    if (!f) {
        DLOG(ERROR) << "no entry on request table for msgid=" << msgid;
        return;
    }
    f->set_result(result, error, z);
}

void session_impl::on_notify(object method, object params, auto_zone z)
{
    // TODO
}

const address& session::get_address() const
{
    return m_pimpl->get_address();
}

loop session::get_loop()
{
    return m_pimpl->get_loop();
}

void session::set_timeout(unsigned int sec)
{
    m_pimpl->set_timeout(sec);
}

unsigned int session::get_timeout() const
{
    return m_pimpl->get_timeout();
}

future session::send_request_impl(msgid_t msgid, std::string method,
    std::auto_ptr<with_shared_zone<vrefbuffer> > vbuf)
{
    return m_pimpl->send_request_impl(msgid, method, vbuf);
}

future session::send_request_impl(msgid_t msgid, std::string method,
    sbuffer* sbuf)
{
    return m_pimpl->send_request_impl(msgid, method, sbuf);
}

void session::send_notify_impl(sbuffer* sbuf)
{
    return m_pimpl->send_notify_impl(sbuf);
}

void session::send_notify_impl(std::auto_ptr<with_shared_zone<vrefbuffer> > vbuf)
{
    return m_pimpl->send_notify_impl(vbuf);
}

msgid_t session::next_msgid()
{
    return m_pimpl->next_msgid();
}


}  // namespace rpc
}  // namespace msgpack
