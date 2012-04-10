#include <stdlib.h>
#include "logpool.h"
#include "logpool_internal.h"

#ifdef __cplusplus
extern "C" {
#endif
struct plugin_entry {
    keyFn     fn_key;
    logapi_t *api;
    void *connection;
};

typedef struct mul {
    int size;
    struct plugin_entry *entries;
} mul_t;

static char *logpool_Multiplexer_fn_key(logctx_t *ctx, uint64_t v, uint64_t seq, sizeinfo_t info)
{
    int i;
    mul_t *mul = cast(mul_t *, ctx->connection);
    for (i = 0; i < mul->size; ++i) {
        struct plugin_entry *e = mul->entries+i;
        cast(struct logctx *, ctx)->connection = e->connection;
        e->fn_key(ctx, v, seq, info);
    }
    cast(struct logctx *, ctx)->connection = mul;
    return NULL;
}

static void *logpool_Multiplexer_init(logctx_t *ctx, logpool_param_t *p)
{
    struct logpool_param_multiplexer *args = cast(struct logpool_param_multiplexer *, p);
    int i, argc = args->argc;
    size_t struct_size = sizeof(mul_t);
    mul_t *mul = cast(mul_t *, malloc(struct_size));
    mul->size  = argc;
    mul->entries = cast(struct plugin_entry *, malloc(sizeof(struct plugin_entry)*argc));
    for (i = 0; i < argc; ++i) {
        struct plugin_param *pa = args->args+i;
        mul->entries[i].api = pa->api;
        mul->entries[i].connection = pa->api->fn_init(ctx, pa->param);
        mul->entries[i].fn_key = ctx->fn_key;
    }
    cast(struct logctx *, ctx)->fn_key = logpool_Multiplexer_fn_key;
    return cast(void *, mul);
}

static void logpool_Multiplexer_flush(logctx_t *ctx, void **args)
{
    int i;
    mul_t *mul = cast(mul_t *, ctx->connection);
    logctx_format_flush(ctx);
    for (i = 0; i < mul->size; ++i) {
        struct plugin_entry *e = mul->entries+i;
        cast(struct logctx *, ctx)->connection = e->connection;
        e->api->fn_flush(ctx, args);
    }
    cast(struct logctx *, ctx)->connection = mul;
}

static void logpool_Multiplexer_close(logctx_t *ctx)
{
    int i;
    mul_t *mul = cast(mul_t *, ctx->connection);
    for (i = 0; i < mul->size; ++i) {
        struct plugin_entry *e = mul->entries+i;
        cast(struct logctx *, ctx)->connection = e->connection;
        e->api->fn_close(ctx);
    }
    free(mul->entries);
    free(mul);
}

static int logpool_Multiplexer_priority(logctx_t *ctx, int priority)
{
    int i, res = 0;
    mul_t *mul = cast(mul_t *, ctx->connection);
    for (i = 0; i < mul->size; ++i) {
        struct plugin_entry *e = mul->entries+i;
        cast(struct logctx *, ctx)->connection = e->connection;
        res |= e->api->fn_priority(ctx, priority);
    }
    cast(struct logctx *, ctx)->connection = mul;
    return res;
}

#define mul_exec(ctx, Fn, key, v, info) do {\
    int i;\
    mul_t *mul = cast(mul_t *, ctx->connection);\
    for (i = 0; i < mul->size; ++i) {\
        struct plugin_entry *e = mul->entries+i;\
        cast(struct logctx *, ctx)->connection = e->connection;\
        e->api->Fn(ctx, key, v, info);\
    }\
    cast(struct logctx *, ctx)->connection = mul;\
} while (0)

static void logpool_Multiplexer_null(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    mul_exec(ctx, fn_null, key, v, info);
}

static void logpool_Multiplexer_bool(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    mul_exec(ctx, fn_bool, key, v, info);
}

static void logpool_Multiplexer_int(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    mul_exec(ctx, fn_int, key, v, info);
}

static void logpool_Multiplexer_hex(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    mul_exec(ctx, fn_hex, key, v, info);
}

static void logpool_Multiplexer_float(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    mul_exec(ctx, fn_float, key, v, info);
}

static void logpool_Multiplexer_char(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    mul_exec(ctx, fn_char, key, v, info);
}

static void logpool_Multiplexer_string(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    mul_exec(ctx, fn_string, key, v, info);
}

static void logpool_Multiplexer_raw(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    mul_exec(ctx, fn_raw, key, v, info);
}

static void logpool_Multiplexer_delim(logctx_t *ctx)
{
    int i;
    mul_t *mul = cast(mul_t *, ctx->connection);
    for (i = 0; i < mul->size; ++i) {
        struct plugin_entry *e = mul->entries+i;
        cast(struct logctx *, ctx)->connection = e->connection;
        e->api->fn_delim(ctx);
    }
    cast(struct logctx *, ctx)->connection = mul;
}

struct logapi MULTIPLEX_API = {
    logpool_Multiplexer_null,
    logpool_Multiplexer_bool,
    logpool_Multiplexer_int,
    logpool_Multiplexer_hex,
    logpool_Multiplexer_float,
    logpool_Multiplexer_char,
    logpool_Multiplexer_string,
    logpool_Multiplexer_raw,
    logpool_Multiplexer_delim,
    logpool_Multiplexer_flush,
    logpool_Multiplexer_init,
    logpool_Multiplexer_close,
    logpool_Multiplexer_priority
};

#ifdef __cplusplus
}
#endif
