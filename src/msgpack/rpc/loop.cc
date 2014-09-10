#include "loop.h"

#include <boost/asio.hpp>

namespace msgpack {
namespace rpc {

loop_impl::loop_impl() :
    m_io(),
#if BOOST_VERSION >= 104800
    m_sigset(m_io),
#endif
    m_timer(m_io)
{
}

loop_impl::~loop_impl()
{
    end();
    join();
}

boost::asio::io_service& loop_impl::io_service()
{
    return m_io;
}

void loop_impl::start(size_t num)
{
    if (is_running()) {
        throw std::runtime_error("loop is already running");
    }
    add_worker(num);
}

void loop_impl::run(size_t num)
{
    if (is_running() == false)
        start(num);
    m_io.run();
}

void loop_impl::run_once()
{
    m_io.run_one();
}

void loop_impl::join()
{
    int size = m_workers.size();
    for (int i = 0; i < size; i++) {
        try {
            m_workers[i]->join();
        } catch (boost::thread_resource_error& e) {
            m_workers[i]->detach();
        }
    }
    m_workers.clear();
}

void loop_impl::end()
{
    m_io.stop();
}

void loop_impl::flush()
{
    m_io.poll();
}

bool loop_impl::is_running()
{
    return !m_workers.empty();
}

void loop_impl::add_worker(size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        std::size_t (boost::asio::io_service::*run_fn)() = &boost::asio::io_service::run;
        std::shared_ptr<boost::thread> thread(new boost::thread(std::bind(run_fn, &m_io)));
        m_workers.push_back(thread);
    }
}

void loop_impl::submit(std::function<void ()> callback)
{
    m_io.post(callback);
}

// timeout

void loop_impl::step_timeout(int sec, std::function<bool ()> handler,
    const boost::system::error_code& err)
{
    if (err) {
        ;
    }
    bool res = handler();
    if (res)
        add_timer(sec, handler);
}

void loop_impl::add_timer(int sec, std::function<bool ()> handler)
{
    m_timer.expires_from_now(boost::posix_time::seconds(sec));
    m_timer.async_wait(std::bind(&loop_impl::step_timeout, this,
            sec, handler, std::placeholders::_1));
}

int loop_impl::add_signal(int signo,
    std::function<void (const boost::system::error_code& error,
        int signo)> handler)
{
#if BOOST_VERSION >= 104800
    m_sigset.add(signo);
    m_sigset.async_wait(handler);
#endif
    return 1;
}

void loop_impl::remove_signal(int id)
{
#if BOOST_VERSION >= 104800
    m_sigset.cancel();
#endif
}

// LOOP
loop::loop() : std::shared_ptr<loop_impl>(new loop_impl())
{
}

loop::~loop()
{
}

}  // namespace rpc
}  // namespace msgpack
