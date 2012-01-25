#include "logpool.h"
#include "logpool_internal.h"
#include "lpstring.h"
#include "jit/llvm.h"
#include <libmemcached/memcached.h>

namespace memcache {
using namespace logpool;

typedef union mc {
    struct jitctx_base base;
    struct _m {
        memcached_st *st;
        logpool::flushFn fn;
    } s;
} mc_t;

static void api_fn_flush(logctx ctx, char *buffer, size_t size __UNUSED__)
{
    //TODO
    assert(0);

    mc_t *mc = cast(mc_t *, ctx->connection);
    char keybuf[128] = {0}, *p;
    const char *key;
    if (ctx->fn_key == logpool_key_hex) {
        p = put_hex(keybuf, ctx->logkey.v.u);
        //TODO check null terminate
        //p[0] = 0;
        key = keybuf;
    } else {
        assert(ctx->fn_key == logpool_key_string);
        key = ctx->logkey.v.s;
    }
    uint32_t flags = 0;
    size_t klen = strlen(key), vlen = strlen(buffer);
    //FIXME vlen == size
    memcached_return_t rc;
    rc = memcached_set(mc->s.st, key, klen, buffer, vlen, 0, flags);
    if (rc != MEMCACHED_SUCCESS) {
        // TODO Error
        abort();
    }
}

void *fn_init(logctx ctx, void **args)
{
    const char *host = cast(const char *, args[1]);
    long port = cast(long, args[2]);
    mc_t *mc = cast(mc_t *, logpool::fn_init(ctx, args));
    memcached_return_t rc;
    memcached_server_list_st servers;

    mc->s.st = memcached_create(NULL);
    mc->s.fn = api_fn_flush;
    if (mc->s.st == NULL) {
        // TODO Error
        abort();
    }
    servers = memcached_server_list_append(NULL, host, port, &rc);
    if (rc != MEMCACHED_SUCCESS) {
        // TODO Error
        abort();
    }
    rc = memcached_server_push(mc->s.st, servers);
    if (rc != MEMCACHED_SUCCESS) {
        // TODO Error
        abort();
    }
    memcached_server_list_free(servers);
    return cast(void *, mc);
}

void fn_close(logctx ctx)
{
    mc_t *mc = cast(mc_t *, ctx->connection);
    memcached_free(mc->s.st);
    logpool::fn_close(ctx);
}

void fn_flush(logctx ctx, void **args)
{
    logpool::fn_flush(ctx, args);
}
} /* namespace memcache */

#ifdef __cplusplus
extern "C" {
#endif

struct logapi LLVM_MEMCACHE_API = {
    logpool::fn_null,
    logpool::fn_bool,
    logpool::fn_int,
    logpool::fn_hex,
    logpool::fn_float,
    logpool::fn_char,
    logpool::fn_string,
    logpool::fn_raw,
    logpool::fn_delim,
    memcache::fn_flush,
    memcache::fn_init,
    memcache::fn_close
};
#ifdef __cplusplus
}
#endif
