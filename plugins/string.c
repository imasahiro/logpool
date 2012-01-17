#include "logpool.h"
#include "logpool_string.h"
#include <stdlib.h>
#include <stdio.h>

void *logpool_string_init(logctx ctx __UNUSED__, void **args)
{
    buffer_t *buf;
    uintptr_t size = cast(uintptr_t, args[0]);
    buf = cast(buffer_t *, malloc(sizeof(*buf) + size - 1));
    buf->buf  = buf->base;
    return cast(void *, buf);
}

void logpool_string_null(logctx ctx, const char *key, uint64_t v __UNUSED__, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    put_string(buf, key, get_l2(info));
    put_string(buf, ":null", 5);
}

void logpool_string_bool(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    const char *s = (v != 0)?":true":":false";
    put_string(buf, key, get_l2(info));
    put_string(buf, s, strlen(s));
}

void logpool_string_int(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    intptr_t i = cast(intptr_t, v);
    put_string(buf, key, get_l2(info));
    put_char(buf, ':');
    buf->buf = write_i(buf->buf, i);
}

void logpool_string_hex(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    if (key) {
        put_string(buf, key, get_l2(info));
        put_char(buf, ':');
    }
    put_char(buf, '0');
    put_char(buf, 'x');
    buf->buf = write_h(buf->buf, v);
}

void logpool_string_float(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    double f = u2f(v);
    put_string(buf, key, get_l2(info));
    put_char(buf, ':');
    buf->buf = write_f(buf->buf, f);
}

void logpool_string_char(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    long c = cast(long, v);
    put_string(buf, key, get_l2(info));
    put_char(buf, ':');
    put_char(buf, (char)c);
}

void logpool_string_string(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    char *s = cast(char *, v);
    if (key) {
        put_string(buf, key, get_l2(info));
        put_char(buf, ':');
    }
    put_char(buf, '\'');
    put_string(buf, s, get_l1(info));
    put_char(buf, '\'');
}

void logpool_string_raw(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    char *s = cast(char *, v);
    put_string(buf, key, get_l2(info));
    put_char(buf, ':');
    put_string(buf, s, get_l1(info));
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
    assert(buf->buf[-1] == '\0');
    buf->buf[-1] = '\n';
    buf->buf[ 0] = '\0';
    fwrite(buf->base, buf->buf - buf->base, 1, stderr);
    buf->buf = buf->base;
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

