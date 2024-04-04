#include "bpf_core/iec/byte_op.h"

u16 bytes::byteswap(u16 x)
{
    union {
        struct
        {
            u8 a;
            u8 b;
        } byteset;
        u16 value;
    } y, z;

    y.value = x;
    z.byteset.a = y.byteset.b;
    z.byteset.b = y.byteset.a;
    return z.value;
}

u32 bytes::byteswap(u32 x)
{
    union {
        struct
        {
            u8 a;
            u8 b;
            u8 c;
            u8 d;
        } byteset;
        u32 value;
    } y, z;

    y.value = x;
    z.byteset.a = y.byteset.d;
    z.byteset.b = y.byteset.c;
    z.byteset.c = y.byteset.b;
    z.byteset.d = y.byteset.a;
    return z.value;
}

u64 bytes::byteswap(u64 x)
{
    union {
        struct
        {
            u8 a;
            u8 b;
            u8 c;
            u8 d;
            u8 e;
            u8 f;
            u8 g;
            u8 h;
        } byteset;
        u64 value;
    } y, z;

    y.value = x;
    z.byteset.a = y.byteset.h;
    z.byteset.b = y.byteset.g;
    z.byteset.c = y.byteset.f;
    z.byteset.d = y.byteset.e;
    z.byteset.e = y.byteset.d;
    z.byteset.f = y.byteset.c;
    z.byteset.g = y.byteset.b;
    z.byteset.h = y.byteset.a;
    return z.value;
}
