//
// msgpack::rpc::address - MessagePack-RPC for C++
//
// Copyright (C) 2009-2010 FURUHASHI Sadayuki
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#include <boost/asio.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/lexical_cast.hpp>
#include "address.h"

namespace msgpack {
namespace rpc {

using namespace boost::asio::ip;

address::address()
{
}

address::address(const boost::asio::ip::address& ip, unsigned short port) :
    m_address(ip),
    m_port(port)
{
}

address::address(const std::string& host, unsigned short port_num)
{
    resolve(host, boost::lexical_cast<std::string>(port_num));
}

address::address(const std::string& addr)
{
    int colon_pos = addr.find(':');
    if (colon_pos < 0) {
        throw std::invalid_argument(std::string("invalid address: ") + addr);
    }
    resolve(addr.substr(0, colon_pos), addr.substr(colon_pos + 1));
}

address::address(const std::string& host, const std::string& service)
{
    resolve(host, service);
}

void address::resolve(const std::string& host, const std::string& service)
{
    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, service);
    tcp::resolver::iterator it = resolver.resolve(query);
    tcp::resolver::iterator end;

    while (it != end) {
        tcp::endpoint ep = it->endpoint();
        if (ep.address().is_v4()) {
            m_address = ep.address();
            m_port = ep.port();
            return;
        }
        ++it;
    }

    throw std::invalid_argument(std::string("can't resolve address: ") + host);
}

bool address::is_v4() const
{
    return m_address.is_v4();
}

boost::asio::ip::address address::get_addr() const
{
    return m_address;
}

unsigned short address::get_port() const
{
    return m_port;
}

bool operator==(const address& a1, const address& a2)
{
    return a1.m_address == a2.m_address && a1.m_port == a2.m_port;
}

bool operator<(const address& a1, const address& a2)
{
    if (a1.m_address == a2.m_address)
        return a1.m_port < a2.m_port;
    return a1.m_address < a2.m_address;
}

std::ostream& operator<< (std::ostream& s, const address& a)
{
    if (a.is_v4()) {
        return s << a.get_addr().to_string() << ':' << a.get_port();
    } else {
        return s << '[' << a.get_addr().to_string() << "]:" << a.get_port();
    }
}

}  // namespace rpc
}  // namespace msgpack

