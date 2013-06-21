#include <iostream>
#include <msgpack/rpc/address.h>

using namespace std;

int main(int argc, char* argv[])
{
    cout << "hostname:service -> localhost:http" << endl;

    msgpack::rpc::address addr("localhost", "http");

    cout << "ip address : " << addr.get_addr().to_string() << endl;
    cout << "port : " << addr.get_port() << endl;
    cout << endl;

    cout << "IP address:port -> 127.0.0.1:80" << endl;
    msgpack::rpc::address addr2("127.0.0.1", 80);

    cout << "ip address : " << addr2.get_addr().to_string() << endl;
    cout << "port : " << addr2.get_port() << endl;
    cout << endl;

    cout << "localhost:http == 127.0.0.1:80" << endl;
    cout << (addr == addr2) << endl;

    msgpack::rpc::address addr3("127.0.0.1", 7070);
    cout << "localhost:http == 127.0.0.1:7070" << endl;
    cout << (addr == addr3) << endl;

    return 0;
}
