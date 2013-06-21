#include "echo_server.h"
#include <iostream>
#include <msgpack/rpc/server.h>
#include <msgpack/rpc/session_pool.h>
#include <glog/logging.h>
#include <signal.h>

void add_callback(rpc::future f, rpc::loop lo)
{
    try {
        int ret = f.get<int>();
        std::cout << "callback: add(1, 2) = " << ret << std::endl;
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        exit(1);
    }
    lo->end();
}

int main(int argc, char **argv)
{
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    google::SetStderrLogging(google::INFO);
    google::LogToStderr();
    signal(SIGPIPE, SIG_IGN);

    // run server {
    rpc::server svr;

    svr.serve(boost::make_shared<myecho>());
    svr.listen("0.0.0.0", 18811);

    svr.start(4);
    // }


    // create session pool
    rpc::session_pool sp;
    // sp.start(4);

    // get session
    rpc::session s = sp.get_session("127.0.0.1", 18811);

    // call
    rpc::future f = s.call("add", 1, 2);

    f.attach_callback(
        boost::bind(add_callback, _1, sp.get_loop()));

    sp.run(4);

    return 0;
}
