#include <stdlib.h>
#include <stdio.h>
#include "logpool.h"
#include "lpstring.h"
#include "logpool_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

void *logpool_string_init(logctx_t *ctx __UNUSED__, logpool_param_t *p)
{
    struct logpool_param_string *args = cast(struct logpool_param_string *, p);
    buffer_t *buf;
    uintptr_t size = cast(uintptr_t, args->buffer_size);
    buf = cast(buffer_t *, malloc(sizeof(*buf) + size - 1));
    buf->buf  = buf->base;
    return cast(void *, buf);
}

void logpool_string_close(logctx_t *ctx)
{
    struct logctx *lctx = cast(struct logctx *, ctx);
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    free(buf);
    lctx->connection = NULL;
}

void logpool_string_null(logctx_t *ctx, const char *key, uint64_t v __UNUSED__, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    buf_put_string(buf, key, get_l2(info));
    buf_put_string(buf, "null", 5);
}

void logpool_string_bool(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    const char *s = (v != 0)?"true":"false";
    size_t len = (v != 0)? 4 : 5;
    buf_put_string(buf, key, get_l2(info));
    buf_put_string(buf, s, len);
}

void logpool_string_int(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    intptr_t i = cast(intptr_t, v);
    buf_put_string(buf, key, get_l2(info));
    buf->buf = put_i(buf->buf, i);
}

void logpool_string_hex(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    buf_put_string(buf, key, get_l2(info));
    buf_put_char2(buf, '0', 'x');
    buf->buf = put_hex(buf->buf, v);
}

void logpool_string_float(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    double f = u2f(v);
    buf_put_string(buf, key, get_l2(info));
    buf->buf = put_f(buf->buf, f);
}

void logpool_string_char(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    long c = cast(long, v);
    buf_put_string(buf, key, get_l2(info));
    buf_put_char(buf, (char)c);
}

void logpool_string_string(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    char *s = cast(char *, v);
    buf_put_string(buf, key, get_l2(info));
    buf_put_char(buf, '\'');
    buf_put_string(buf, s, get_l1(info));
    buf_put_char(buf, '\'');
}

void logpool_string_raw(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    char *s = cast(char *, v);
    buf_put_string(buf, key, get_l2(info));
    buf_put_string(buf, s, get_l1(info));
}

void logpool_string_delim(logctx_t *ctx)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    buf_put_char(buf, ',');
}

void logpool_string_flush(logctx_t *ctx)
{
    logpool_string_flush_internal(ctx);
}

static void logpool_string_flush__(logctx_t *ctx, void **args __UNUSED__)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    logpool_string_flush(ctx);
    assert(buf->buf[-1] == '\0');
    put_char2(buf->buf-1, '\n', '\0');
    fwrite(buf->base, buf->buf - buf->base, 1, stderr);
    logpool_string_reset(ctx);
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
    logpool_string_close,
};

static char *write_seq(buffer_t *buf, uint64_t seq)
{
    buf_put_char(buf, '+');
    buf->buf = put_hex(buf->buf, seq);
    return buf->buf;
}

char *logpool_key_hex(logctx_t *ctx, uint64_t v, uint64_t seq, sizeinfo_t info __UNUSED__)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    buf_put_char2(buf, '0' ,'x');
    buf->buf = put_hex(buf->buf, v);
    return write_seq(buf, seq);
}

char *logpool_key_string(logctx_t *ctx, uint64_t v, uint64_t seq, sizeinfo_t info)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    char *s = cast(char *, v);
    buf_put_string(buf, s, get_l2(info));
    return write_seq(buf, seq);
}

static struct keyapi STRING_KEY_API = {
    logpool_key_hex,
    logpool_key_string
};

struct keyapi *logpool_string_api_init(void)
{
    return &STRING_KEY_API;
}


#ifdef __cplusplus
}
#endif
