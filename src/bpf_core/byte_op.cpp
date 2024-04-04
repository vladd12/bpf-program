#include "bpf_core/byte_op.h"

ui16 bytes::byteswap(ui16 x)
{
    union {
        struct
        {
            ui8 a;
            ui8 b;
        } byteset;
        ui16 value;
    } y, z;

    y.value = x;
    z.byteset.a = y.byteset.b;
    z.byteset.b = y.byteset.a;
    return z.value;
}

ui32 bytes::byteswap(ui32 x)
{
    union {
        struct
        {
            ui8 a;
            ui8 b;
            ui8 c;
            ui8 d;
        } byteset;
        ui32 value;
    } y, z;

    y.value = x;
    z.byteset.a = y.byteset.d;
    z.byteset.b = y.byteset.c;
    z.byteset.c = y.byteset.b;
    z.byteset.d = y.byteset.a;
    return z.value;
}

ui64 bytes::byteswap(ui64 x)
{
    union {
        struct
        {
            ui8 a;
            ui8 b;
            ui8 c;
            ui8 d;
            ui8 e;
            ui8 f;
            ui8 g;
            ui8 h;
        } byteset;
        ui64 value;
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
