#ifndef LOGPOOL_STRING_H_
#define LOGPOOL_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct buffer {
    char *buf;
    void *unused;
    char base[1];
} buffer_t;

void *logpool_string_init(logctx ctx, void **param);
void logpool_string_null(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void logpool_string_bool(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void logpool_string_int(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void logpool_string_hex(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void logpool_string_float(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void logpool_string_char(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void logpool_string_string(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void logpool_string_raw(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void logpool_string_delim(logctx ctx);
void logpool_string_flush(logctx ctx);

void logpool_string_reset(logctx ctx);

void logpool_key_string(logctx ctx, uint64_t v, uint64_t seq, sizeinfo_t info);
void logpool_key_hex(logctx ctx, uint64_t v, uint64_t seq, sizeinfo_t info);

#define PTR_SIZE (sizeof(void*))
#define BITS (PTR_SIZE * 8)
#define CLZ(n) __builtin_clzl(n)
static inline char *put_hex(char *const start, uint64_t v)
{
    static const char __digit__[] = "0123456789abcdef";
    register char *p = start;
    int i = (BITS - CLZ(v) - 1) >> 2 << 2;
    for (; i >= 0; i -= 4) {
        int c = 0xf & (v >> i);
        *(p++) = __digit__[c];
    }
    return p;
}

static void reverse(char *const start, char *const end, const int len)
{
    int i, l = len / 2;
    register char *s = start;
    register char *e = end - 1;
    for (i = 0; i < l; i++) {
        char tmp = *s;
        tmp  = *s;
        *s++ = *e;
        *e-- = tmp;
    }
}

static inline char *put_d(char *const p, uint64_t uvalue)
{
    int i = 0;
    static const char _t_[] = "0123456789abcdef";
    while (uvalue != 0) {
        int r = uvalue % 10;
        p[i]  = _t_[r];
        uvalue /= 10;
        i++;
    }
    reverse(p, p + i, i);
    return p + i;
}

static inline char *put_i(char *p, intptr_t value)
{
    if(value < 0) {
        p[0] = '-'; p++;
        value = -value;
    }
    uintptr_t u = value / 10, r = value % 10;
    if (u != 0) {
        p = put_d(p, u);
    }
    p[0] = ('0' + r);
    return p + 1;
}

static inline char *put_f(char *p, double f)
{
    intptr_t value = (intptr_t) (f*1000);
    if(value < 0) {
        p[0] = '-'; p++;
        value = -value;
    }
    intptr_t u = value / 1000, r = value % 1000;
    if(u != 0) {
        p = put_d(p, u);
    }
    else {
        p[0] = '0'; p++;
    }
    p[0] = '.'; p++;
    u = r / 100;
    r = r % 100;
    p[0] = ('0' + (u)); p++;
    p[0] = ('0' + (r / 10)); p++;
    p[0] = ('0' + (r % 10));
    return p + 1;
}

static inline void put_string(buffer_t *buf, const char *s, short size)
{
    int i;
    char *p = buf->buf;
    for (i = 0; i < size; ++i) {
        p[i] = s[i];
    }
    buf->buf += size;
}

static inline void put_char(buffer_t *buf, char c)
{
    buf->buf[0] = c;
    ++(buf->buf);
}

static inline void put_char2(buffer_t *buf, char c0, char c1)
{
    buf->buf[0] = c0;
    buf->buf[1] = c1;
    buf->buf += 2;
}

static inline void put_char3(buffer_t *buf, char c0, char c1, char c2)
{
    buf->buf[0] = c0;
    buf->buf[1] = c1;
    buf->buf[2] = c2;
    buf->buf += 3;
}

static inline short get_l1(sizeinfo_t info)
{
    return (info >> sizeof(short)*8);
}
static inline short get_l2(sizeinfo_t info)
{
    return (short)info;
}

#ifdef __cplusplus
}
#endif

#endif /* end of include guard */
