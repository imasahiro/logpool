#include <stdlib.h>
#include "logpool.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

static struct keyapi *KeyAPI = NULL;
extern struct logapi SYSLOG_API;
extern struct logapi FILE2_API;
extern struct logapi MEMCACHE_API;

void logctx_init(logctx ctx, struct logapi *api, struct logpool_param *param)
{
    struct logCtx *lctx = cast(struct logCtx *, ctx);
    lctx->logfmt_capacity = param->logfmt_capacity;
    lctx->fmt = cast(logfmt_t *, malloc(sizeof(logfmt_t) * lctx->logfmt_capacity));
    lctx->formatter   = api;
    lctx->connection  = api->fn_init(ctx, param);
    lctx->logkey.k.seq = 0;
    lctx->logfmt_size  = 0;
}

void logctx_format_flush(logctx ctx)
{
    struct logfmt *fmt = cast(struct logCtx *, ctx)->fmt;
    size_t i, size = ctx->logfmt_size;
    ctx->fn_key(ctx, ctx->logkey.v.u, ctx->logkey.k.seq, ctx->logkey.siz);
    if (size) {
        void (*fn_delim)(logctx) = ctx->formatter->fn_delim;
        /* unroled */
        fn_delim(ctx);
        fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
        ++fmt;
        for (i = 1; i < size; ++i, ++fmt) {
            fn_delim(ctx);
            fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
        }
        cast(struct logCtx *, ctx)->logfmt_size = 0;
    }
    ++(cast(struct logCtx *, ctx)->logkey.k.seq);
}

void logctx_append_fmtdata(logctx ctx, const char *key, uint64_t v, logFn f, sizeinfo_t siz)
{
    struct logCtx *lctx = cast(struct logCtx *, ctx);
    assert(lctx->logfmt_size < lctx->logfmt_capacity);
    lctx->fmt[lctx->logfmt_size].fn    = f;
    lctx->fmt[lctx->logfmt_size].k.key = key;
    lctx->fmt[lctx->logfmt_size].v.u   = v;
    lctx->fmt[lctx->logfmt_size].siz   = siz;
    ++lctx->logfmt_size;
}

void logctx_init_logkey(logctx ctx, uint64_t v, sizeinfo_t siz)
{
    struct logCtx *lctx = cast(struct logCtx *, ctx);
    lctx->logkey.v.u = v;
    lctx->logkey.siz = siz;
    lctx->logfmt_size = 0;
}

ltrace_t *ltrace_open(ltrace_t *parent, struct logapi *api, struct logpool_param *p)
{
    struct ltrace *l = cast(struct ltrace *, malloc(sizeof(*l)));
    logctx_init(cast(logctx, l), api, p);
    l->parent = parent;
    l->ctx.fn_key = KeyAPI->str;
    return cast(ltrace_t*, l);
}

ltrace_t *ltrace_open_syslog(ltrace_t *parent)
{
    struct logpool_param_syslog param = {8, 1024};
    extern struct logapi SYSLOG_API;
    return ltrace_open(parent, &SYSLOG_API, (struct logpool_param*) &param);
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
    cast(logctx, l)->formatter->fn_close(cast(logctx, l));
    free(l);
}

static uint64_t hash(uint64_t h, const char *p, size_t len)
{
    size_t i;
    for(i = 0; i < len; i++) {
        h = p[i] + 31 * h;
    }
    return h;
}

lstate_t *lstate_open(const char *state_name, struct logapi *api, struct logpool_param *p)
{
    struct lstate *l = cast(struct lstate *, malloc(sizeof(*l)));
    l->state = hash(0x11029, state_name, strlen(state_name));
    logctx_init(cast(logctx, l), api, p);
    l->ctx.fn_key = KeyAPI->hex;
    return cast(lstate_t*, l);
}

lstate_t *lstate_open_syslog(const char *state)
{
    struct logpool_param_syslog param = {8, 1024};
    return lstate_open(state, &SYSLOG_API, (struct logpool_param*) &param);
}

lstate_t *lstate_open_file(const char *state, char *filename)
{
    struct logpool_param_file param = {8, 1024};
    param.fname = (const char *) filename;
    return lstate_open(state, &FILE2_API, (struct logpool_param *) &param);
}

lstate_t *lstate_open_memcache(const char *state, char *host, long ip)
{
    struct logpool_param_memcache param = {8, 1024};
    param.host = host;
    param.port = ip;
    return lstate_open(state, &MEMCACHE_API, (struct logpool_param *) &param);
}

void lstate_close(lstate_t *p)
{
    struct lstate *l = cast(struct lstate *, p);
    cast(logctx, l)->formatter->fn_close(cast(logctx, l));
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
}

#ifdef __cplusplus
}
#endif
