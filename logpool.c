#include "logpool.h"
#include <stdlib.h>

void logctx_init(logctx ctx, struct logapi *api, void **param)
{
    struct logctx *lctx = cast(struct logctx *, ctx);
    lctx->formatter   = api;
    lctx->connection  = api->fn_init(ctx, param);
    lctx->logfmt_size = 0;
}

void logctx_format_flush(logctx ctx)
{
    struct logfmt *fmt = cast(struct logctx *, ctx)->fmt;
    size_t i, size = ctx->logfmt_size;
    ctx->logkey.fn(ctx, NULL, ctx->logkey.v.u, ctx->logkey.siz);
    ctx->formatter->fn_delim(ctx);
    if (size) {
        fmt->fn(ctx, fmt->key, fmt->v.u, fmt->siz);
        fmt++;
        for (i = 1; i < size; ++i, ++fmt) {
            ctx->formatter->fn_delim(ctx);
            fmt->fn(ctx, fmt->key, fmt->v.u, fmt->siz);
        }
        cast(struct logctx *, ctx)->logfmt_size = 0;
    }
}

void logctx_append_fmtdata(logctx ctx, const char *key, uint64_t v, logFn f, sizeinfo_t siz)
{
    struct logctx *lctx = cast(struct logctx *, ctx);
    assert(lctx->logfmt_size < LOGFMT_MAX_SIZE);
    lctx->fmt[lctx->logfmt_size].fn  = f;
    lctx->fmt[lctx->logfmt_size].key = key;
    lctx->fmt[lctx->logfmt_size].v.u = v;
    lctx->fmt[lctx->logfmt_size].siz = siz;
    ++lctx->logfmt_size;
}

void logctx_init_logkey(logctx ctx, uint64_t v, logFn f, size_t size)
{
    struct logctx *lctx = cast(struct logctx *, ctx);
    lctx->logkey.fn  = f;
    lctx->logkey.v.u = v;
    lctx->logkey.siz = size;
    lctx->logfmt_size = 0;
}

ltrace_t *ltrace_open(ltrace_t *parent, struct logapi *api, void **param)
{
    struct ltrace *l = cast(struct ltrace *, malloc(sizeof(*l)));
    logctx_init(cast(logctx, l), api, param);
    l->parent = parent;
    return cast(ltrace_t*, l);
}

void ltrace_close(ltrace_t *p)
{
    struct ltrace *l = cast(struct ltrace *, p);
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

lstate_t *lstate_open(const char *state_name, struct logapi *api, void **param)
{
    struct lstate *l = cast(struct lstate *, malloc(sizeof(*l)));
    l->state = hash(0x11029, state_name, strlen(state_name));
    logctx_init(cast(logctx, l), api, param);
    return cast(lstate_t*, l);
}

void lstate_close(lstate_t *p)
{
    struct lstate *l = cast(struct lstate *, p);
    free(l);
}


