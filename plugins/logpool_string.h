#ifndef LOGPOOL_STRING_H_
#define LOGPOOL_STRING_H_

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

static inline char *write_d(char *const p, uint64_t uvalue, int radix)
{
    int i = 0;
    while (uvalue != 0) {
        int tmp = uvalue % radix;
        uvalue /= radix;
        p[i] = (tmp < 10 ? '0' + tmp : + 'a' + tmp - 10);
        ++i;
    }
    reverse(p, p + i, i);
    return p + i;
}

static inline char *write_i(char *p, intptr_t value)
{
    if(value < 0) {
        p[0] = '-'; p++;
        value = -value;
    }
    uintptr_t u = value / 10, r = value % 10;
    if(u != 0) {
        p = write_d(p, u, 10);
    }
    p[0] = ('0' + r);
    return p + 1;
}

static inline char *write_f(char *p, double f)
{
    intptr_t value = (intptr_t) (f*1000);
    if(value < 0) {
        p[0] = '-'; p++;
        value = -value;
    }
    intptr_t u = value / 1000, r = value % 1000;
    if(u != 0) {
        p = write_d(p, u, 10);
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

static inline short get_l1(sizeinfo_t info)
{
    return (info >> sizeof(short)*8);
}
static inline short get_l2(sizeinfo_t info)
{
    return (short)info;
}

#endif /* end of include guard */
