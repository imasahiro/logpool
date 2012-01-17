#include "logpool.h"
#include "logpool_string.h"
#include <stdlib.h>
#include <stdio.h>

static inline void put_string(buffer_t *buf, const char *s)
{
    size_t len = strlen(s);
    memcpy(buf->buf, s, len);
    buf->buf += len;
}

static inline void put_char(buffer_t *buf, char c)
{
    buf->buf[0] = c;
    ++(buf->buf);
}

void *logpool_string_init(logctx ctx __UNUSED__, void **args)
{
    buffer_t *buf;
    uintptr_t size = cast(uintptr_t, args[0]);
    buf = cast(buffer_t *, malloc(sizeof(*buf) + size - 1));
    buf->buf  = buf->base;
    buf->ebuf = buf->buf + size;
    return cast(void *, buf);
}

void logpool_string_null(logctx ctx, const char *key, uint64_t v __UNUSED__)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    put_string(buf, key);
    put_string(buf, ":null");
}

void logpool_string_bool(logctx ctx, const char *key, uint64_t v)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    const char *s = (v != 0)?":true":":false";
    put_string(buf, key);
    put_string(buf, s);
}

void logpool_string_int(logctx ctx, const char *key, uint64_t v)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    intptr_t i = cast(intptr_t, v);
    put_string(buf, key);
    put_char(buf, ':');
    buf->buf = write_i(buf->buf, buf->ebuf, i);
}

void logpool_string_hex(logctx ctx, const char *key, uint64_t v)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    if (key) {
        put_string(buf, key);
        put_char(buf, ':');
    }
    put_string(buf, "0x");
    buf->buf = write_d(buf->buf, buf->ebuf, v, 16);
}

void logpool_string_float(logctx ctx, const char *key, uint64_t v)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    double f = u2f(v);
    put_string(buf, key);
    put_char(buf, ':');
    buf->buf = write_f(buf->buf, buf->ebuf, f);
}

void logpool_string_char(logctx ctx, const char *key, uint64_t v)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    long c = cast(long, v);
    put_string(buf, key);
    put_char(buf, ':');
    put_char(buf, (char)c);
}

void logpool_string_string(logctx ctx, const char *key, uint64_t v)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    char *s = cast(char *, v);
    if (key) {
        put_string(buf, key);
        put_char(buf, ':');
    }
    put_char(buf, '\'');
    put_string(buf, s);
    put_char(buf, '\'');
}

void logpool_string_raw(logctx ctx, const char *key, uint64_t v)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    char *s = cast(char *, v);
    put_string(buf, key);
    put_char(buf, ':');
    put_string(buf, s);
}

void logpool_string_delim(logctx ctx)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    put_char(buf, ',');
}

void logpool_string_flush(logctx ctx)
{
    logctx_format_flush(ctx);
    {
        buffer_t *buf = cast(buffer_t *, ctx->connection);
        put_char(buf, 0);
    }
}

static void logpool_string_flush__(logctx ctx)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    logpool_string_flush(ctx);
    fwrite(buf->base, buf->buf - buf->base - 1, 1, stderr);
}

struct logapi STRING_API = {
    logpool_string_null,
    logpool_string_bool,
    logpool_string_int,
    logpool_string_hex,
    logpool_string_float,
    logpool_string_char,
    logpool_string_string,
    logpool_string_raw,
    logpool_string_delim,
    logpool_string_flush__,
    logpool_string_init,
};

