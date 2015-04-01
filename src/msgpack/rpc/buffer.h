//
// msgpack::rpc::buffer - Cluster Communication Framework
//
#ifndef MSGPACK_RPC_BUFFER_H__
#define MSGPACK_RPC_BUFFER_H__

#include <boost/shared_array.hpp>
#include <msgpack_fwd.hpp>

namespace msgpack {
namespace rpc {


// introduced to handle read()/write() with one interface
//
// refer - ByteBuffer.cpp

struct buffer {
    uint32_t m_size;
    const char* m_ptr;
    boost::shared_array<char> s_ptr;

    buffer() : m_size(0), m_ptr(NULL), s_ptr() { }
    buffer(const char* p, uint32_t s) : m_size(s), m_ptr(p), s_ptr() { }

    ~buffer() {
        clear();
    }

    int32_t size() const {
        return m_size;
    }

    const char* data() const {
        return m_ptr;
    }

    void wrap(const char* p, uint32_t s);
    void make_unwrapped();
    void clear();
};

template <typename Stream>
inline packer<Stream>& operator<< (packer<Stream>& o, const buffer& v)
{
    o.pack_bin(v.m_size);
    o.pack_bin_body(v.m_ptr, v.m_size);
    return o;
}

buffer& operator>> (object o, buffer& v);

inline void operator<< (object& o, const buffer& v)
{
    o.type = type::BIN;
    o.via.bin.ptr = v.m_ptr;
    o.via.bin.size = v.m_size;
}

inline void operator<< (object::with_zone& o, const buffer& v)
{
    assert(0);
}


}  // namespace rpc
}  // namespace msgpack

#endif /* msgpack/rpc/buffer.h */
