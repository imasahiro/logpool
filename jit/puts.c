#include <stdint.h>
static void put_char2(char *p, int8_t c0, int8_t c1)
{
    uint16_t v = (c1 << 8) | c0;
    *(uint16_t*)p = v;
}

static void put_char4(char *p, int8_t c0, int8_t c1, int8_t c2, int8_t c3)
{
    uint32_t v = (c3 << 24) | (c2 << 16) | (c1 << 8) | c0;
    *(uint32_t*)p = v;
}

#define BITS (sizeof(void*) * 8)
#define CLZ(n) __builtin_clzl(n)
#define ALIGN(x,n)  (((x)+((n)-1))&(~((n)-1)))
static inline char *put_hex(char *const start, uint64_t v)
{
    static const char __digit__[] = "0123456789abcdef";
    register char *p = start;
    int i = ALIGN((v>0?(BITS-CLZ(v)):1), 4) - 4;
    do {
        unsigned char c = 0xf & (v >> i);
        *(p++) = __digit__[c];
    } while ((i -= 4) >= 0);
    return p;
}

static void reverse(char *const start, char *const end)
{
    char *m = start + (end - start) / 2;
    char tmp, *s = start, *e = end - 1;
    while (s < m) {
        tmp  = *s;
        *s++ = *e;
        *e-- = tmp;
    }
}

static inline char *put_d(char *p, uint64_t v)
{
    char *base = p;
    do {
        *p++ = '0' + ((uint8_t)(v % 10));
    } while ((v /= 10) != 0);

    reverse(base, p);
    return p;
}

static inline char *put_i(char *p, int64_t value)
{
    if(value < 0) {
        p[0] = '-'; p++;
        value = -value;
    }
    return put_d(p, (uint64_t)value);
}

static inline char *put_f(char *p, double f)
{
    intptr_t value = (intptr_t) (f*1000);
    if(value < 0) {
        p[0] = '-'; p++;
        value = -value;
    }
    int32_t u = value / 1000, r = value % 1000;
    int32_t s1, s2, s3;
    p = put_d(p, u);
    s3 = r % 100;
    u  = r / 100;
    /* s2 = s3 / 10; s1 = s3 % 10 */
    s2 = (s3 * 0xcd) >> 11;
    s1 = (s3) - 10*s2;
    put_char4(p, '.', ('0' + u), ('0' + s2), ('0' + s1));
    return p + 4;
}


char *llvm_put_h(char *p, uintptr_t value)
{
    return put_hex(p, value);
}

char *llvm_put_i(char *p, intptr_t value)
{
    return put_i(p, value);
}

char *llvm_put_f(char *p, double f)
{
    return put_f(p, f);
}

