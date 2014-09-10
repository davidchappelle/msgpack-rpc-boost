#include "echo_server.h"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/make_shared.hpp>
#include <iostream>
#include <msgpack/rpc/server.h>
#include <msgpack/rpc/session_pool.h>
#include <signal.h>

int main(int argc, char **argv)
{
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
    signal(SIGPIPE, SIG_IGN);

    // run server {
    rpc::server svr;
    svr.serve(boost::make_shared<myecho>());

    svr.listen("0.0.0.0", 18811);
    svr.start(4);
    // }


    // create session pool
    rpc::session_pool sp;

    // get session
    rpc::session s = sp.get_session("127.0.0.1", 18811);

    // async call
    rpc::future fs[10];

    for (int i=0; i < 10; ++i) {
        fs[i] = s.call("add", 1, 2);
    }

    for (int i=0; i < 10; ++i) {
        int ret = fs[i].get<int>();
        std::cout << "async call: add(1, 2) = " << ret << std::endl;
    }

    return 0;
}
