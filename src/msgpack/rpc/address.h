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
#ifndef MSGPACK_RPC_ADDRESS_H__
#define MSGPACK_RPC_ADDRESS_H__

#include <boost/asio.hpp>

namespace msgpack {
namespace rpc {

class address
{
public:
    address();
    address(const std::string& host, unsigned short port);
    address(const std::string& host, const std::string& service);
    address(const std::string& addr);  // hostname:port

	bool is_v4() const;
    boost::asio::ip::address get_addr() const;
    unsigned short get_port() const;

    void resolve(const std::string& host, const std::string& service);

	friend bool operator==(const address& a1, const address& a2);
	friend bool operator<(const address& a1, const address& a2);

private:
    boost::asio::ip::address m_address;
    uint16_t m_port;
};

std::ostream& operator<< (std::ostream& stream, const address& a);

}  // namespace rpc
}  // namespace msgpack

#endif /* msgpack/rpc/address.h */
