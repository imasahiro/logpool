#include "logpool.h"
#include "logpool_string.h"
#include <libmemcached/memcached.h>

typedef struct mc {
    char *buf;
    char *ebuf;
    memcached_st *st;
    char base[1];
} mc_t;


void *logpool_memcache_init(logctx ctx, void *param)
{
    void **args = cast(void **, param);
    mc_t *mc = cast(mc_t *, logpool_string_init(ctx, (void*) param[0]));
    const char* host = cast(const char *, param[1]);
    int port = cast(int, param[2]);
    memcached_return_t rc;
    memcached_server_list_st servers;

    mc->st = memcached_create(NULL);
    if (mcd->st == NULL) {
        // TODO Error
    }
    servers = memcached_server_list_append(NULL, host, port, &rc);
    if (rc != MEMCACHED_SUCCESS) {
        // TODO Error
    }
    rc = memcached_server_push(mcd->st, servers);
    if (rc != MEMCACHED_SUCCESS) {
        // TODO Error
    }
    memcached_server_list_free(servers);
    return cast(void *, mc);
}

static void logpool_memcache_flush(logctx ctx)
{
    mc_t *mc = cast(mc_t *, ctx->connection);
    logpool_string_flush(ctx);
    const char *key = String_to(const char*, sfp[1]);
    const char *value = String_to(const char *, sfp[2]);
    size_t klen = strlen(key);
    size_t vlen = strlen(mc->base);
    uint32_t flags = 0;
    memcached_return_t rc;
    rc = memcached_set(mc->st, key, klen, value, vlen, 0, flags);
    if (rc != MEMCACHED_SUCCESS) {
        // TODO Error
    }
}

struct logapi SYSLOG_API = {
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

