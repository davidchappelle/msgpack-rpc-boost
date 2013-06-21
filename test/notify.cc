#include "echo_server.h"
#include <msgpack/rpc/server.h>
#include <msgpack/rpc/client.h>
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
    svr.listen("0.0.0.0", 18811);

    svr.start(4);
    // }


    // create client
    rpc::client cli("127.0.0.1", 18811);

    // notify
    std::string one("one"), two("two");
    cli.notify("oneway");
    cli.notify("oneway", one);
    cli.notify("oneway", two, one);

    cli.get_loop()->flush();
    usleep(100000);

    return 0;
}

