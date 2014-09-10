#include "echo_server.h"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <memory>
#include <msgpack/rpc/server.h>
#include <msgpack/rpc/client.h>
#include <signal.h>

int main(int argc, char **argv)
{
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
    signal(SIGPIPE, SIG_IGN);

    // run server {
    rpc::server svr;

    svr.serve(std::make_shared<myecho>());
    svr.listen("0.0.0.0", 18811);

    svr.start(4);
    // }


    // create client
    rpc::client cli("127.0.0.1", 18811);

    // call
    std::string msg("MessagePack-RPC");
    std::string ret = cli.call("echo", msg).get<std::string>();

    std::cout << "call: echo(\"MessagePack-RPC\") = " << ret << std::endl;

    return 0;
}

