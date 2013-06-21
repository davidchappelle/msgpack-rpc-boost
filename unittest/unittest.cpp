#include <gtest/gtest.h>
#include <glog/logging.h>

#include "echo_server.h"
#include "msgpack/rpc/client.h"
#include "msgpack/rpc/server.h"
#include "msgpack/rpc/exception.h"
#include "msgpack/rpc/transport/tcp.h"
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>

GTEST_API_ int main(int argc, char **argv)
{
    printf("Running main() from unittest.cc\n");
    testing::InitGoogleTest(&argc, argv);
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);
    google::SetStderrLogging(google::INFO);
    google::LogToStderr();
    return RUN_ALL_TESTS();
}

TEST(EchoServer, Add)
{
    using namespace msgpack;
    using namespace msgpack::rpc;

    try {
        const int PORT = 18811;
        msgpack::rpc::server server;

        server.serve(boost::make_shared<myecho>());
        server.listen("0.0.0.0", PORT);
        server.start(5);

        msgpack::rpc::client cli("127.0.0.1", PORT);

        int a = -12;
        int b = 13;
        future cc = cli.call("add", a, b);
        int i = cc.get<int>();
        EXPECT_EQ((a+b), i);

    } catch(const argument_error& e) {
        ADD_FAILURE() << "argument_error exception " << e.what();
    }
    catch (const boost::system::system_error& e)
    {
        ADD_FAILURE() << e.what();
    }
    catch (...)
    {
        ADD_FAILURE() << "Uncaught exception";
    }
}


TEST(EchoServer, IncorrectPort)
{
    using namespace msgpack;
    using namespace msgpack::rpc;
    using namespace boost;

    try {
        const int PORT = 18811;
        msgpack::rpc::server server;

        server.serve(boost::make_shared<myecho>());
        server.listen("0.0.0.0", PORT);

        msgpack::rpc::client cli("127.0.0.1", 16296);
        cli.call("add", 1, 2).get<int>();
    } catch(const argument_error& e) {
        ADD_FAILURE() << "argument_error exception " << e.what();
    }
    catch(const connect_error& e)
    {
        SUCCEED();
        return;
    }
    catch (...)
    {
        FAIL() << "Uncaught exception";
    }
    ADD_FAILURE() << "Didn't throw exception as expected";
}

TEST(EchoServer, NoMethodError)
{
    using namespace msgpack;
    using namespace msgpack::rpc;
    using namespace boost;

    try {
        const int PORT = 18811;
        msgpack::rpc::server server;

        server.serve(boost::make_shared<myecho>());
        server.listen("0.0.0.0", PORT);
        server.start(1);

        msgpack::rpc::client cli("127.0.0.1", PORT);
        cli.call("sub", 1, 2).get<int>();
    } catch(const argument_error& e) {
        ADD_FAILURE() << "argument_error exception " << e.what();
    }
    catch (const no_method_error&)
    {
        SUCCEED();
        return;
    }
    catch (...)
    {
        FAIL() << "Uncaught exception";
    }
    ADD_FAILURE() << "Didn't throw exception as expected";
}

TEST(EchoServer, TimeoutErrorSession)
{
    using namespace msgpack;
    using namespace msgpack::rpc;
    using namespace boost;

    try {
        const int PORT = 18811;
        msgpack::rpc::server server;

        server.serve(boost::make_shared<myecho>());
        server.listen("0.0.0.0", PORT);
        server.start(1);

        rpc::session_pool sp;
        sp.start(1);
        rpc::session s = sp.get_session("127.0.0.1", PORT);
        s.set_timeout(3);
        s.call("timeout").get<int>();
    }
    catch (const timeout_error&)
    {
        SUCCEED();
        return;
    }
    catch (std::exception& e)
    {
        FAIL() << "Uncaught exception";
    }
}

TEST(EchoServer, TimeoutErrorClient)
{
    using namespace msgpack;
    using namespace msgpack::rpc;
    using namespace boost;

    try {
        const int PORT = 18811;
        msgpack::rpc::server server;

        server.serve(boost::make_shared<myecho>());
        server.listen("0.0.0.0", PORT);
        server.start(1);

        msgpack::rpc::client cli("127.0.0.1", PORT);
        cli.set_timeout(3);
        cli.call("timeout").get<int>();
    }
    catch (const timeout_error&)
    {
        SUCCEED();
        return;
    }
    catch (std::exception& e)
    {
        FAIL() << "Uncaught exception";
    }
}

TEST(EchoServer, Err)
{
    using namespace msgpack;
    using namespace msgpack::rpc;
    using namespace boost;

    try {
        const int PORT = 18811;
        msgpack::rpc::server server;

        server.serve(boost::make_shared<myecho>());
        server.listen("0.0.0.0", PORT);
        server.start(5);

        msgpack::rpc::client cli("127.0.0.1", PORT);
        cli.call("err").get<int>();
    }
    catch (const std::exception&)
    {
        SUCCEED();
        return;
    }
    catch (...)
    {
        FAIL() << "Uncaught exception";
        return;
    }
    ADD_FAILURE() << "Didn't throw exception as expected";
}

TEST(EchoServer, Notify)
{
    using namespace msgpack;
    using namespace msgpack::rpc;
    using namespace boost;

    try {
        const int PORT = 18811;
        msgpack::rpc::server server;

        server.serve(boost::make_shared<myecho>());
        server.listen("0.0.0.0", PORT);
        server.start(5);

        msgpack::rpc::client cli("127.0.0.1", PORT);
        cli.notify("echo");
        cli.get_loop()->flush();
    } catch(const argument_error& e) {
        ADD_FAILURE() << "argument_error exception " << e.what();
    }
    catch (const boost::system::system_error& e)
    {
        ADD_FAILURE() << "system_error exception " << e.what();
    }
    catch (...)
    {
        FAIL() << "Uncaught exception";
    }
}
