#include "logpool.h"
#include "logpool_internal.h"
#include "lpstring.h"
#include <libmemcached/memcached.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mc {
    char *buf;
    memcached_st *st;
    char base[1];
} mc_t;

void *logpool_memcache_init(logctx ctx, void **param)
{
    const char *host = cast(const char *, param[1]);
    long port = cast(long, param[2]);
    mc_t *mc = cast(mc_t *, logpool_string_init(ctx, param));
    memcached_return_t rc;
    memcached_server_list_st servers;

    mc->st = memcached_create(NULL);
    if (unlikely(mc->st == NULL)) {
        /* TODO Error */
        abort();
    }
    servers = memcached_server_list_append(NULL, host, port, &rc);
    if (unlikely(rc != MEMCACHED_SUCCESS)) {
        /* TODO Error */
        abort();
    }
    rc = memcached_server_push(mc->st, servers);
    if (unlikely(rc != MEMCACHED_SUCCESS)) {
        /* TODO Error */
        abort();
    }
    memcached_server_list_free(servers);
    return cast(void *, mc);
}

static void logpool_memcache_flush(logctx ctx, void **args __UNUSED__)
{
    mc_t *mc = cast(mc_t *, ctx->connection);
    char key[128] = {0}, *buf_orig = mc->buf, *p;
    const char *value = mc->base;
    uint32_t flags = 0;
    size_t klen, vlen;
    memcached_return_t rc;
    struct logfmt *fmt = cast(struct logCtx *, ctx)->fmt;
    size_t i, size = ctx->logfmt_size;

    mc->buf = key;
    p = ctx->fn_key(ctx, ctx->logkey.v.u, ctx->logkey.k.seq, ctx->logkey.siz);
    klen = p - (char*) key;
    mc->buf = buf_orig;

    if (size) {
        void (*fn_delim)(logctx) = ctx->formatter->fn_delim;
        fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
        ++fmt;
        for (i = 1; i < size; ++i, ++fmt) {
            fn_delim(ctx);
            fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
        }
        cast(struct logCtx *, ctx)->logfmt_size = 0;
    }

    vlen = (char*) mc->buf - value;
    rc = memcached_set(mc->st, key, klen, value, vlen, 0, flags);
    if (unlikely(rc != MEMCACHED_SUCCESS)) {
        // TODO Error
        abort();
    }
    logpool_string_reset(ctx);
    ++(cast(struct logCtx *, ctx)->logkey.k.seq);
}

struct logapi MEMCACHE_API = {
    logpool_string_null,
    logpool_string_bool,
    logpool_string_int,
    logpool_string_hex,
    logpool_string_float,
    logpool_string_char,
    logpool_string_string,
    logpool_string_raw,
    logpool_string_delim,
    logpool_memcache_flush,
    logpool_memcache_init,
};

#ifdef __cplusplus
}
#endif
