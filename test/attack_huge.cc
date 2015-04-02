#include "attack.h"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <signal.h>

static size_t ATTACK_SIZE;
static size_t ATTACK_THREAD;
static size_t ATTACK_LOOP;

static std::unique_ptr<attacker> test;

using msgpack::type::raw_ref;

static rpc::shared_zone msglife;
static raw_ref msg;

void attack_huge()
{
    rpc::client c(test->builder(), test->address());
    c.set_timeout(60.0);

    for(size_t i=0; i < ATTACK_LOOP; ++i) {
        raw_ref result = c.call("echo_huge", msglife, msg).get<raw_ref>();

        if(result.size != msg.size) {
            BOOST_LOG_TRIVIAL(error) << "invalid size: " << result.size;
        } else if(memcmp(result.ptr, msg.ptr, msg.size) != 0) {
            BOOST_LOG_TRIVIAL(error) << "received data don't match with sent data.";
        }
    }
}

int main(int argc, char **argv)
{
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
    signal(SIGPIPE, SIG_IGN);

    ATTACK_SIZE   = attacker::option("SIZE",   1024*1024, 4*1024*1024);
    ATTACK_THREAD = attacker::option("THREAD", 5, 20);
    ATTACK_LOOP   = attacker::option("LOOP",   2, 20);

    std::cout << "huge send/recv attack"
        << " size="   << (ATTACK_SIZE/1024/1024) << "MB"
        << " thread=" << ATTACK_THREAD
        << " loop="   << ATTACK_LOOP
        << std::endl;

    msglife.reset(new msgpack::zone());
    msg = raw_ref((char*)msglife->allocate_align(ATTACK_SIZE), ATTACK_SIZE);
    memset((char *)msg.ptr, 0, ATTACK_SIZE);

    test.reset(new attacker());
    test->run(ATTACK_THREAD, &attack_huge);

    return 0;
}

