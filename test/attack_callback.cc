#include "attack.h"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <functional>
#include <iostream>
#include <signal.h>
#include <vector>

static size_t ATTACK_DEPTH;
static size_t ATTACK_THREAD;
static size_t ATTACK_LOOP;
static size_t ATTACK_SIZE = 1024;

static std::unique_ptr<attacker> test;
static std::unique_ptr<rpc::session_pool> sp;

using msgpack::type::raw_ref;

void callback_func(rpc::future f, raw_ref msg, rpc::shared_zone msglife)
{
    raw_ref result = f.get<raw_ref>();

    if(result.size != msg.size) {
        BOOST_LOG_TRIVIAL(error) << "invalid size: " << result.size;
    } else if(memcmp(result.ptr, msg.ptr, msg.size) != 0) {
        BOOST_LOG_TRIVIAL(error) << "received data don't match with sent data.";
    }
}

void attack_callback()
{
    std::vector<rpc::future> pipeline(ATTACK_DEPTH);

    for(size_t i=0; i < ATTACK_LOOP; ++i) {
        rpc::session s = sp->get_session(test->address());
        s.set_timeout(30.0);

        rpc::shared_zone msglife(new msgpack::zone());
        raw_ref msg = raw_ref((char*)msglife->malloc(ATTACK_SIZE), ATTACK_SIZE);
        memset((char *)msg.ptr, 0, ATTACK_SIZE);

        for(size_t j=0; j < ATTACK_DEPTH; ++j) {
            pipeline[j] = s.call("echo_huge", msglife, msg);
            pipeline[j].attach_callback(
                    std::bind(callback_func, std::placeholders::_1, msg, msglife));
        }

        for(size_t j=0; j < ATTACK_DEPTH; ++j) {
            pipeline[j].wait(false);
        }
    }
}

int main(int argc, char **argv)
{
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
    signal(SIGPIPE, SIG_IGN);

    ATTACK_DEPTH  = attacker::option("DEPTH",  25, 100);
    ATTACK_THREAD = attacker::option("THREAD", 25, 100);
    ATTACK_LOOP   = attacker::option("LOOP",   5, 50);

    std::cout << "callback attack"
        << " depth="  << ATTACK_DEPTH
        << " thread=" << ATTACK_THREAD
        << " loop="   << ATTACK_LOOP
        << std::endl;

    test.reset(new attacker());

    sp.reset(new rpc::session_pool(test->builder()));
    sp->start(4);

    test->run(ATTACK_THREAD, &attack_callback);

    return 0;
}
