#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "logpool.h"
#include "logpool_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

static struct keyapi *KeyAPI = NULL;
extern struct logapi SYSLOG_API;
extern struct logapi FILE2_API;
extern struct logapi MEMCACHE_API;

static void logctx_init(logctx_t *ctx, struct logapi *api, logpool_param_t *param)
{
    struct logctx *lctx = cast(struct logctx *, ctx);
    lctx->logfmt_capacity = param->logfmt_capacity;
    lctx->fmt = cast(logfmt_t *, malloc(sizeof(logfmt_t) * lctx->logfmt_capacity));
    lctx->formatter   = api;
    lctx->connection  = api->fn_init(ctx, param);
    lctx->logkey.k.seq = 0;
    lctx->logfmt_size  = 0;
    lctx->is_flushed   = 0;
}

static void logctx_close(logctx_t *ctx)
{
    struct logctx *lctx = cast(struct logctx *, ctx);
    free(lctx->fmt);
    lctx->fmt = NULL;
}

void logctx_format_flush(logctx_t *ctx)
{
    struct logfmt *fmt = cast(struct logctx *, ctx)->fmt;
    size_t i, size = ctx->logfmt_size;
    if (ctx->is_flushed)
        return;
    cast(struct logctx *, ctx)->is_flushed = 1;
    ctx->fn_key(ctx, ctx->logkey.v.u, ctx->logkey.k.seq, ctx->logkey.siz);
    if (size) {
        void (*fn_delim)(logctx_t *) = ctx->formatter->fn_delim;
        /* unroled */
        fn_delim(ctx);
        fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
        ++fmt;
        for (i = 1; i < size; ++i, ++fmt) {
            fn_delim(ctx);
            fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
        }
        cast(struct logctx *, ctx)->logfmt_size = 0;
    }
    ++(cast(struct logctx *, ctx)->logkey.k.seq);
}

void logctx_flush(logctx_t *ctx, void **args)
{
    cast(struct logctx *, ctx)->is_flushed = 0;
    ctx->formatter->fn_flush(ctx, args);
}

void logctx_append_fmtdata(logctx_t *ctx, const char *key, uint64_t v, logFn f, sizeinfo_t siz)
{
    struct logctx *lctx = cast(struct logctx *, ctx);
    assert(lctx->logfmt_size < lctx->logfmt_capacity);
    lctx->fmt[lctx->logfmt_size].fn    = f;
    lctx->fmt[lctx->logfmt_size].k.key = key;
    lctx->fmt[lctx->logfmt_size].v.u   = v;
    lctx->fmt[lctx->logfmt_size].siz   = siz;
    ++lctx->logfmt_size;
}

int logctx_init_logkey(logctx_t *ctx, int priority, uint64_t v, sizeinfo_t siz)
{
    int doLog = ctx->formatter->fn_priority(ctx, priority);
    if (doLog) {
        struct logctx *lctx = cast(struct logctx *, ctx);
        lctx->logkey.v.u = v;
        lctx->logkey.siz = siz;
        lctx->logfmt_size = 0;
    }
    return doLog;
}

ltrace_t *ltrace_open(ltrace_t *parent, struct logapi *api, logpool_param_t *p)
{
    struct ltrace *l = cast(struct ltrace *, malloc(sizeof(*l)));
    l->parent = parent;
    l->ctx.fn_key = KeyAPI->str;
    logctx_init(cast(logctx_t *, l), api, p);
    return l;
}

ltrace_t *ltrace_open_syslog(ltrace_t *parent)
{
    struct logpool_param_syslog param = {8, 1024};
    return ltrace_open(parent, &SYSLOG_API, (logpool_param_t *) &param);
}

ltrace_t *ltrace_open_file(ltrace_t *parent, char *filename)
{
    struct logpool_param_file param = {8, 1024};
    param.fname = (const char *) filename;
    return ltrace_open(parent, &FILE2_API,  (struct logpool_param*) &param);
}

ltrace_t *ltrace_open_memcache(ltrace_t *parent, char *host, long ip)
{
    struct logpool_param_memcache param = {8, 1024};
    param.host = host;
    param.port = ip;
    return ltrace_open(parent, &MEMCACHE_API, (struct logpool_param*) &param);
}

void ltrace_close(ltrace_t *p)
{
    struct ltrace *l = cast(struct ltrace *, p);
    cast(logctx_t *, l)->formatter->fn_close(cast(logctx_t *, l));
    logctx_close(cast(logctx_t *, l));
    free(l);
}

#ifdef LOGPOOL_USE_LLVM
extern struct keyapi *logpool_llvm_api_init(void);
#endif
extern struct keyapi *logpool_string_api_init(void);
void logpool_init(enum LOGPOOL_EXEC_MODE mode)
{
    if (mode == LOGPOOL_JIT) {
#ifdef LOGPOOL_USE_LLVM
        KeyAPI = logpool_llvm_api_init();
#else
        assert(0 && "please enable USE_LLVM flag");
#endif
    } else {
        KeyAPI = logpool_string_api_init();
    }
}

void logpool_exit(void)
{
    /* TODO */
}

#ifdef __cplusplus
}
#endif
