#include "echo_server.h"
#include <iostream>
#include <msgpack/rpc/server.h>
#include <msgpack/rpc/client.h>
#include <msgpack/rpc/transport/udp.h>
#include <glog/logging.h>
#include <signal.h>

int main(int argc, char **argv)
{
    google::InitGoogleLogging(argv[0]);
    google::SetStderrLogging(google::INFO);
    google::LogToStderr();
    signal(SIGPIPE, SIG_IGN);

    // run server {
    rpc::server svr;

    svr.serve(boost::make_shared<myecho>());
    svr.listen( msgpack::rpc::udp_listener("0.0.0.0", 18811) );

    svr.start(4);
    // }


    // create client
    rpc::client cli(msgpack::rpc::udp_builder(), msgpack::rpc::address("127.0.0.1", 18811));

    // call
    std::string msg("MessagePack-RPC");
    std::string ret = cli.call("echo", msg).get<std::string>();

    std::cout << "call: echo(\"MessagePack-RPC\") = " << ret << std::endl;

    return 0;
}

