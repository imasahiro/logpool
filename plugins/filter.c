#include <stdlib.h>
#include "logpool.h"
#include "logpool_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct filter {
    int   priority;
    void *connection;
    logapi_t *api;
    keyFn     fn_key;
} filter_t;

static char *logpool_Filter_fn_key(logctx_t *ctx, uint64_t v, uint64_t seq, sizeinfo_t info)
{
    char *ret;
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logctx *, ctx)->connection = filter->connection;
    asm volatile("int3");
    ret = filter->fn_key(ctx, v, seq, info);
    cast(struct logctx *, ctx)->connection = filter;
    return ret;
}

static void *logpool_Filter_init(logctx_t *ctx, logpool_param_t *p)
{
    struct logpool_param_filter *args = cast(struct logpool_param_filter *, p);
    filter_t *filter = cast(filter_t *, malloc(sizeof(*filter)));
    filter->api = args->api;
    filter->priority   = args->priority;
    filter->connection = args->api->fn_init(ctx, &args->param);
    filter->fn_key = ctx->fn_key;
    cast(struct logctx *, ctx)->fn_key = logpool_Filter_fn_key;
    return cast(void *, filter);
}

static void logpool_Filter_close(logctx_t *ctx)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logctx *, ctx)->connection = filter->connection;
    filter->api->fn_close(ctx);
    free(filter);
}

static void logpool_Filter_flush(logctx_t *ctx, void **args __UNUSED__)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    logctx_format_flush(ctx);
    cast(struct logctx *, ctx)->connection = filter->connection;
    filter->api->fn_flush(ctx, args);
    cast(struct logctx *, ctx)->connection = filter;
}

static int logpool_Filter_priority(logctx_t *ctx, int priority)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    return (filter->priority < priority);
}

static void logpool_Filter_null(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logctx *, ctx)->connection = filter->connection;
    filter->api->fn_null(ctx, key, v, info);
    cast(struct logctx *, ctx)->connection = filter;
}

static void logpool_Filter_bool(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logctx *, ctx)->connection = filter->connection;
    filter->api->fn_bool(ctx, key, v, info);
    cast(struct logctx *, ctx)->connection = filter;
}

static void logpool_Filter_int(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logctx *, ctx)->connection = filter->connection;
    filter->api->fn_int(ctx, key, v, info);
    cast(struct logctx *, ctx)->connection = filter;
}

static void logpool_Filter_hex(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logctx *, ctx)->connection = filter->connection;
    filter->api->fn_hex(ctx, key, v, info);
    cast(struct logctx *, ctx)->connection = filter;
}

static void logpool_Filter_float(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logctx *, ctx)->connection = filter->connection;
    filter->api->fn_float(ctx, key, v, info);
    cast(struct logctx *, ctx)->connection = filter;
}

static void logpool_Filter_char(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logctx *, ctx)->connection = filter->connection;
    filter->api->fn_char(ctx, key, v, info);
    cast(struct logctx *, ctx)->connection = filter;
}

static void logpool_Filter_string(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logctx *, ctx)->connection = filter->connection;
    filter->api->fn_string(ctx, key, v, info);
    cast(struct logctx *, ctx)->connection = filter;
}

static void logpool_Filter_raw(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logctx *, ctx)->connection = filter->connection;
    filter->api->fn_raw(ctx, key, v, info);
    cast(struct logctx *, ctx)->connection = filter;
}

static void logpool_Filter_delim(logctx_t *ctx)
{
    filter_t *filter = cast(filter_t *, ctx->connection);
    cast(struct logctx *, ctx)->connection = filter->connection;
    filter->api->fn_delim(ctx);
    cast(struct logctx *, ctx)->connection = filter;
}

struct logapi FILTER_API = {
    logpool_Filter_null,
    logpool_Filter_bool,
    logpool_Filter_int,
    logpool_Filter_hex,
    logpool_Filter_float,
    logpool_Filter_char,
    logpool_Filter_string,
    logpool_Filter_raw,
    logpool_Filter_delim,
    logpool_Filter_flush,
    logpool_Filter_init,
    logpool_Filter_close,
    logpool_Filter_priority
};

#ifdef __cplusplus
}
#endif
