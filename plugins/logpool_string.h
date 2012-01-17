#ifndef LOGPOOL_STRING_H_
#define LOGPOOL_STRING_H_

typedef struct buffer {
    char *buf;
    char *ebuf;
    void *unused;
    char base[1];
} buffer_t;

void *logpool_string_init(logctx ctx, void *param);
void logpool_string_null(logctx ctx, const char *key, uint64_t v);
void logpool_string_bool(logctx ctx, const char *key, uint64_t v);
void logpool_string_int(logctx ctx, const char *key, uint64_t v);
void logpool_string_hex(logctx ctx, const char *key, uint64_t v);
void logpool_string_float(logctx ctx, const char *key, uint64_t v);
void logpool_string_char(logctx ctx, const char *key, uint64_t v);
void logpool_string_string(logctx ctx, const char *key, uint64_t v);
void logpool_string_raw(logctx ctx, const char *key, uint64_t v);
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

static inline char *write_d(char *const p, const char *const end, uint64_t uvalue, const int base)
{
    int i = 0;
    while (p + i < end) {
        int tmp = uvalue % base;
        uvalue /= base;
        p[i] = tmp + (char)((base == 16 && tmp >= 10)?('a'-10):'0');
        ++i;
        if (uvalue == 0)
            break;
    }
    reverse(p, p + i, i);
    return p + i;
}

static inline char *write_i(char *p, char *ebuf, intptr_t value)
{
    if(value < 0) {
        p[0] = '-'; p++;
        value = -value;
    }
    uintptr_t u = value / 10, r = value % 10;
    if(u != 0) {
        p = write_d(p, ebuf, u, 10);
    }
    p[0] = ('0' + r);
    return p + 1;
}

static inline char *write_f(char *p, char *ebuf, double f)
{
    intptr_t value = (intptr_t) (f*1000);
    if(value < 0) {
        p[0] = '-'; p++;
        value = -value;
    }
    intptr_t u = value / 1000, r = value % 1000;
    if(u != 0) {
        p = write_d(p, ebuf, u, 10);
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

#endif /* end of include guard */
