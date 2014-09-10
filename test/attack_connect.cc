#include "attack.h"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <signal.h>

static size_t ATTACK_THREAD;
static size_t ATTACK_LOOP;

static std::unique_ptr<attacker> test;

void attack_connect()
{
    for(size_t i=0; i < ATTACK_LOOP; ++i) {
        rpc::client c(test->builder(), test->address());
        c.set_timeout(30.0);

        int result = c.call("add", 1, 2).get<int>();
        if(result != 3) {
            BOOST_LOG_TRIVIAL(error) << "invalid response: " << result;
        }
    }
}

int main(int argc, char **argv)
{
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
    signal(SIGPIPE, SIG_IGN);

    ATTACK_THREAD = attacker::option("THREAD", 25, 100);
    ATTACK_LOOP   = attacker::option("LOOP", 5, 50);

    std::cout << "connect attack"
        << " thread=" << ATTACK_THREAD
        << " loop="   << ATTACK_LOOP
        << std::endl;

    test.reset(new attacker());
    test->run(ATTACK_THREAD, &attack_connect);

    return 0;
}

