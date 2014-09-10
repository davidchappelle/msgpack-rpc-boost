#include "echo_server.h"

#include <boost/log/trivial.hpp>
#include <boost/thread.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <msgpack/rpc/server.h>
#include <msgpack/rpc/client.h>
#include <msgpack/rpc/transport/tcp.h>
#include <msgpack/rpc/transport/udp.h>
#include <numeric>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>


class attacker {
public:
	attacker() : m_nthreads(0)
	{
		unsigned short port = 18800;
		const char* env_port = getenv("TEST_PORT");
		if(env_port) {
			int i = atoi(env_port);
			if(i) {
				port = i;
			}
		}

		const char* env_proto = getenv("TEST_PROTO");
		if(env_proto && strcmp(env_proto, "udp") == 0) {
			m_listen_addr = rpc::address("0.0.0.0", port);
			m_listener.reset(new rpc::udp_listener(m_listen_addr));

			m_connect_addr = rpc::address("127.0.0.1", port);
			m_builder.reset(new rpc::udp_builder());
		} else {
			m_listen_addr = rpc::address("0.0.0.0", port);
			m_listener.reset(new rpc::tcp_listener(m_listen_addr));

			m_connect_addr = rpc::address("127.0.0.1", port);
			m_builder.reset(new rpc::tcp_builder());
		}
	}

	~attacker() { }

	rpc::builder& builder() { return *m_builder; }
	rpc::address address() { return m_connect_addr; }

	void start_server(size_t nthreads=4)
	{
		m_svr.reset(new rpc::server(*m_builder));
		m_dp.reset(new myecho());

		m_svr->serve(m_dp);
		m_svr->listen(*m_listener);

		m_svr->start(nthreads);
	}

	void start_attacker(size_t nthreads, std::function<void ()> func)
	{
		m_threads.resize(nthreads);
		m_times.resize(nthreads);
		m_nthreads = nthreads;

		for(size_t i=0; i < m_nthreads; ++i) {
			m_times[i] = 0;
			m_threads[i] = new boost::thread(
					std::bind(attacker::thread_main, func, &m_times[i]));
		}
	}

	void join_attacker()
	{
		for(size_t i=0; i < m_nthreads; ++i) {
			//m_threads[i].join();
			m_threads[i]->join();
            delete m_threads[i];
		}
		m_svr->end();
	}

	void join_server()
	{
		m_svr->join();
		m_svr.reset();
	}

	void show_status() const
	{
		double sum = std::accumulate(m_times.begin(), m_times.end(), 0.0);
		double ave = sum / m_nthreads;

		std::vector<double> var_tmp(m_times.size());
		for(size_t i=0; i < var_tmp.size(); ++i) {
			double a = ave - m_times[i];
			var_tmp[i] = a*a;
		}
		double var = std::accumulate(var_tmp.begin(), var_tmp.end(), 0.0) / m_nthreads;

		std::cout
			<< "total time    : " << sum << "\n"
			<< "variance      : " << var << std::endl;
	}

	void run(size_t nthreads, std::function<void ()> func)
	{
		start_server();
		start_attacker(nthreads, func);
		join_attacker();
		show_status();
		join_server();
	}

private:
	rpc::address m_listen_addr;
	std::unique_ptr<rpc::listener> m_listener;

	rpc::address m_connect_addr;
	std::unique_ptr<rpc::builder> m_builder;

	std::unique_ptr<rpc::server> m_svr;
	std::shared_ptr<rpc::dispatcher> m_dp;

	size_t m_nthreads;
	std::vector<boost::thread*> m_threads;
	std::vector<double> m_times;

	static void thread_main(std::function<void ()> func, double* time)
	{
		struct timeval start_time;
		gettimeofday(&start_time, NULL);

		try {
			func();
		} catch (std::exception& e) {
			BOOST_LOG_TRIVIAL(error) << "error: " << e.what();
		} catch (...) {
			BOOST_LOG_TRIVIAL(error) << "error: unknown error";
		}

		struct timeval end_time;
		gettimeofday(&end_time, NULL);

		*time = (end_time.tv_sec - start_time.tv_sec)
			+ (double)(end_time.tv_usec - start_time.tv_usec) / 1000 / 1000;
	}

private:
	attacker(const attacker&);

public:
	static size_t option(const char* name, size_t light, size_t heavy)
	{
		char* env_value = getenv(name);
		if(env_value) {
			int i = atoi(env_value);
			if(i != 0) {
				return i;
			}
		}

		char* env_heavy = getenv("HEAVY_TEST");
		if(env_heavy && strcmp(env_heavy, "0") != 0 && strcmp(env_heavy, "false") != 0) {
			return heavy;
		} else {
			return light;
		}
	}
};

