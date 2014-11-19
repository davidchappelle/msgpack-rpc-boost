#include "loop.h"

#include <boost/asio.hpp>

namespace msgpack {
namespace rpc {

loop_impl::loop_impl() :
    m_io_service(),
    m_workers()
{
}

loop_impl::~loop_impl()
{
    end();
    join();
}

boost::asio::io_service& loop_impl::io_service()
{
    return m_io_service;
}

void loop_impl::start(size_t num)
{
    if (num > 0)
    {
        if (is_running()) {
            throw std::runtime_error("loop is already running");
        }
        add_worker(num);
    }
}

void loop_impl::run(size_t num)
{
    if (is_running() == false)
        start(num);
    m_io_service.run();
}

void loop_impl::run_once()
{
    m_io_service.run_one();
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
    m_io_service.stop();
}

void loop_impl::flush()
{
    m_io_service.poll();
}

bool loop_impl::is_running()
{
    return !m_workers.empty();
}

void loop_impl::add_worker(size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        std::size_t (boost::asio::io_service::*run_fn)() = &boost::asio::io_service::run;
        std::shared_ptr<boost::thread> thread(new boost::thread(std::bind(run_fn, &m_io_service)));
        m_workers.push_back(thread);
    }
}

void loop_impl::submit(std::function<void ()> callback)
{
    m_io_service.post(callback);
}

loop::loop() : std::shared_ptr<loop_impl>(new loop_impl())
{
}

loop::~loop()
{
}

}  // namespace rpc
}  // namespace msgpack
