//
// msgpack::rpc::buffer - Cluster Communication Framework
//

#include "buffer.h"

#include <assert.h>
#include <msgpack.hpp>

namespace msgpack {
namespace rpc {

// FIXME- reduce memcopy using zone
//
// read - 3 memcpy occurs (2 is redundent)
// 1. read buffer -> unpack buffer
// 2. unpack buffer -> object
// 3. object -> msgpack::rpc::buffer convert

buffer& operator>> (object o, buffer& v)
{
    if (o.type != type::BIN) {
        throw type_error();
    }
    // memory copy occurs
    v.wrap(o.via.bin.ptr, o.via.bin.size);
    v.make_unwrapped();
    return v;
}

void buffer::wrap(const char* p, uint32_t s)
{
    if (s_ptr.get())
        s_ptr.reset();
    m_size = s;
    m_ptr = p;
}

void buffer::make_unwrapped()
{
    if (s_ptr.get())
        return;
    if (m_ptr && m_size) {
        s_ptr.reset(new char[m_size]);
        memcpy(s_ptr.get(), m_ptr, m_size);
        m_ptr = s_ptr.get();
    }
    else {
        clear();
    }
}

void buffer::clear()
{
    if (s_ptr.get())
        s_ptr.reset();
    m_size = 0;
    m_ptr = NULL;
}


}  // namespace rpc
}  // namespace msgpack
