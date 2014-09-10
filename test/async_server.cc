#include "echo_server.h"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <msgpack/rpc/server.h>
#include <msgpack/rpc/client.h>
#include <signal.h>

namespace rpc {
    using namespace msgpack::rpc;
}

class myproxy : public rpc::dispatcher
{
public:
    myproxy(msgpack::rpc::server *svr) {
        m_svr = svr;
    }

    static void callback(rpc::future f, rpc::request req)
    {
        req.result(f.get<int>());
    }

    virtual void dispatch(rpc::request req)
    {
        std::string method = req.method().as<std::string>();
        msgpack::type::tuple<int, int> params(req.params());

        rpc::session s = m_svr->get_session("127.0.0.1", 18811);
        rpc::future f = s.call(method, params.get<0>(), params.get<1>());
        f.attach_callback(std::bind(&callback, std::placeholders::_1, req));
    }
    
private:
    msgpack::rpc::server *m_svr;
};

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


    // run pxocy server {
    rpc::server proxy;

    proxy.serve(std::make_shared<myproxy>(&proxy));
    proxy.listen("0.0.0.0", 18812);

    proxy.start(4);
    // }

    // send rquest from the client
    msgpack::rpc::client c("127.0.0.1", 18812);
    int ret = c.call("add", 1, 2).get<int>();

    std::cout << "call: add(1, 2) = " << ret << std::endl;
}
