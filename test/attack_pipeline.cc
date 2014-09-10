#include "attack.h"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <signal.h>
#include <vector>

static size_t ATTACK_DEPTH;
static size_t ATTACK_THREAD;
static size_t ATTACK_LOOP;

static std::unique_ptr<attacker> test;
static std::unique_ptr<rpc::session_pool> sp;

void attack_pipeline()
{
    std::vector<rpc::future> pipeline(ATTACK_DEPTH);

    for(size_t i=0; i < ATTACK_LOOP; ++i) {
        rpc::session s = sp->get_session(test->address());
        s.set_timeout(30.0);

        for(size_t j=0; j < ATTACK_DEPTH; ++j) {
            pipeline[j] = s.call("add", 1, 2);
        }

        for(size_t j=0; j < ATTACK_DEPTH; ++j) {
            int result = pipeline[j].get<int>();
            if(result != 3) {
                BOOST_LOG_TRIVIAL(error) << "invalid response: " << result;
            }
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

    std::cout << "pipeline attack"
        << " depth="  << ATTACK_DEPTH
        << " thread=" << ATTACK_THREAD
        << " loop="   << ATTACK_LOOP
        << std::endl;

    test.reset(new attacker());

    sp.reset(new rpc::session_pool(test->builder()));
    sp->start(4);

    test->run(ATTACK_THREAD, &attack_pipeline);

    return 0;
}
