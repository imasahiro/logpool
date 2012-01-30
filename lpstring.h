#ifndef LOGPOOL_STRING_H_
#define LOGPOOL_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BITS (sizeof(void*) * 8)
#define CLZ(n) __builtin_clzl(n)
#define ALIGN(x,n)  (((x)+((n)-1))&(~((n)-1)))

typedef struct buffer {
    char *buf;
    void *unused;
    char base[1];
} buffer_t;

static inline void put_char2(char *p, int8_t c0, int8_t c1)
{
    uint16_t v = (c1 << 8) | c0;
    *(uint16_t*)p = v;
}

static inline void put_char4(char *p, int8_t c0, int8_t c1, int8_t c2, int8_t c3)
{
    uint32_t v = (c3 << 24) | (c2 << 16) | (c1 << 8) | c0;
    *(uint32_t*)p = v;
}

static inline void buf_put_char(buffer_t *buf, char c)
{
    buf->buf[0] = c;
    ++(buf->buf);
}

static inline void buf_put_char2(buffer_t *buf, char c0, char c1)
{
    put_char2(buf->buf, c0, c1);
    buf->buf += 2;
}

static inline void buf_put_char3(buffer_t *buf, char c0, char c1, char c2)
{
    put_char4(buf->buf, c0, c1, c2, 0);
    buf->buf += 3;
}

void *logpool_string_init(logctx ctx, void **param);
void logpool_string_close(logctx ctx);
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
char *logpool_key_string(logctx ctx, uint64_t v, uint64_t seq, sizeinfo_t info);
char *logpool_key_hex(logctx ctx, uint64_t v, uint64_t seq, sizeinfo_t info);

static inline void logpool_string_flush_internal(logctx ctx)
{
    buffer_t *buf;
    logctx_format_flush(ctx);
    buf = cast(buffer_t *, ctx->connection);
    buf_put_char(buf, 0);
}

static inline void logpool_string_reset(logctx ctx)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    buf->buf = buf->base;
}

static inline char *put_hex(char *const start, uint64_t v)
{
    static const char __digit__[] = "0123456789abcdef";
    register char *p = start;
    int i = ALIGN((v>0?(BITS-CLZ(v)):4), 4) - 4;
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
    int32_t s1, s2, s3, u, r;
    intptr_t value = (intptr_t) (f*1000);
    if(value < 0) {
        p[0] = '-'; p++;
        value = -value;
    }
    u = value / 1000;
    r = value % 1000;
    p = put_d(p, u);
    s3 = r % 100;
    u  = r / 100;
    /* s2 = s3 / 10; s1 = s3 % 10 */
    s2 = (s3 * 0xcd) >> 11;
    s1 = (s3) - 10*s2;
    put_char4(p, '.', ('0' + u), ('0' + s2), ('0' + s1));
    return p + 4;
}

static inline char *put_string(char *p, const char *s, short size)
{
    const char *e = s + size;
    while (s < e) {
        *p++ = *s++;
    }
    return p;
}

static inline void buf_put_string(buffer_t *buf, const char *s, short size)
{
    buf->buf = put_string(buf->buf, s, size);
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
