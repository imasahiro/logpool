#include <stdlib.h>
#include <stdarg.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "logpool.h"
#include "logpool_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

static struct keyapi *KeyAPI = NULL;

static void logpool_new(logpool_t *ctx, struct logapi *api, logpool_param_t *param)
{
    struct logpool *lctx = cast(struct logpool *, ctx);
    lctx->logfmt_capacity = param->logfmt_capacity;
    lctx->fmt = cast(logfmt_t *, malloc(sizeof(logfmt_t) * lctx->logfmt_capacity));
    lctx->formatter   = api;
    lctx->connection  = api->fn_init(ctx, param);
    lctx->logkey.k.seq = 0;
    lctx->logfmt_size  = 0;
    lctx->is_flushed   = 0;
}

static void append_fmtdata(logpool_t *ctx, const char *key, uint64_t v, logFn f, short klen, short vlen)
{
    struct logpool *lctx = cast(struct logpool *, ctx);
    assert(lctx->logfmt_size < lctx->logfmt_capacity);
    lctx->fmt[lctx->logfmt_size].fn    = f;
    lctx->fmt[lctx->logfmt_size].k.key = key;
    lctx->fmt[lctx->logfmt_size].v.u   = v;
    lctx->fmt[lctx->logfmt_size].klen  = klen;
    lctx->fmt[lctx->logfmt_size].vlen  = vlen;
    ++lctx->logfmt_size;
}

static void logpool_flush(logpool_t *ctx, void *args)
{
    cast(struct logpool *, ctx)->is_flushed = 0;
    ctx->formatter->fn_flush(ctx, args);
}

/* @see konoha2/logger.h */
static const int logfn_index[] = {
    /* LOG_END */ -1,
    /* LOG_s */    6,
    /* LOG_u */    2,
    /* LOG_i */    2,
    /* LOG_b */    1,
    /* LOG_f */    4,
    /* LOG_x */    3,
    /* LOG_n */    0,
    /* LOG_r */    7,
};

void logpool_record(logpool_t *ctx, void *args, int priority, char *trace_id, ...)
{
    va_list ap;
    va_start(ap, trace_id);
    long klen, vlen;
    char *key, *val;
    int i, type;
    logFn f;

    for (i = 0; i < ctx->logfmt_capacity; ++i) {
        type = va_arg(ap, int);
        if (type == 0)
            break;
        klen = va_arg(ap, long);
        vlen = va_arg(ap, long);
        key  = va_arg(ap, char *);
        val  = va_arg(ap, char *);
        f = ((logFn*)ctx->formatter)[logfn_index[type]];
        append_fmtdata(ctx, key, (uint64_t)val, f, klen, vlen);
    }
    va_end(ap);
    logpool_flush(ctx, args);
}

void logpool_format_flush(logpool_t *ctx)
{
    struct logfmt *fmt = cast(struct logpool *, ctx)->fmt;
    size_t i, size = ctx->logfmt_size;
    if (ctx->is_flushed)
        return;
    ctx->fn_key(ctx, ctx->logkey.v.u, ctx->logkey.k.seq, ctx->logkey.klen);
    if (size) {
        void (*fn_delim)(logpool_t *) = ctx->formatter->fn_delim;
        /* unroled */
        fn_delim(ctx);
        fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->klen, fmt->vlen);
        ++fmt;
        for (i = 1; i < size; ++i, ++fmt) {
            fn_delim(ctx);
            fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->klen, fmt->vlen);
        }
        cast(struct logpool *, ctx)->logfmt_size = 0;
    }
    ++(cast(struct logpool *, ctx)->logkey.k.seq);
    cast(struct logpool *, ctx)->is_flushed = 1;
}

int logpool_init_logkey(logpool_t *ctx, int priority, uint64_t v, short klen, short vlen)
{
    int emitLog = ctx->formatter->fn_priority(ctx, priority);
    if (emitLog) {
        struct logpool *lctx = cast(struct logpool *, ctx);
        lctx->logkey.v.u = v;
        lctx->logkey.klen = klen;
        lctx->logkey.vlen = vlen;
        lctx->logfmt_size = 0;
    }
    return emitLog;
}

logpool_t *logpool_open(logpool_t *parent, struct logapi *api, logpool_param_t *p)
{
    struct logpool *l = cast(struct logpool *, malloc(sizeof(*l)));
    l->parent = parent;
    l->fn_key = KeyAPI->str;
    logpool_new(cast(logpool_t *, l), api, p);
    return l;
}

void logpool_close(logpool_t *p)
{
    struct logpool *l = cast(struct logpool *, p);
    cast(logpool_t *, l)->formatter->fn_close(cast(logpool_t *, l));
    free(l->fmt);
    l->fmt = NULL;
    free(l);
}

#ifdef LOGPOOL_USE_LLVM
extern struct keyapi *logpool_llvm_api_init(void);
#endif
extern struct keyapi *logpool_string_api_init(void);
extern struct keyapi *logpool_event_api_init(void);
static int exec_mode = 0;

void logpool_init(int mode)
{
    assert(exec_mode == 0);
    exec_mode = mode;
#if 0
    if (mode == LOGPOOL_JIT) {
#ifdef LOGPOOL_USE_LLVM
        KeyAPI = logpool_llvm_api_init();
#else
        assert(0 && "please enable USE_LLVM flag");
#endif
    } else if (mode == LOGPOOL_EVENT) {
        KeyAPI = logpool_event_api_init();
    } else {
        KeyAPI = logpool_string_api_init();
    }
#else
    KeyAPI = logpool_string_api_init();
#endif
}

extern void logpool_event_api_deinit(void);
void logpool_exit(void)
{
    assert(exec_mode != 0);
#if 0
    if (exec_mode == LOGPOOL_EVENT) {
        logpool_event_api_deinit();
    }
#endif
    exec_mode = 0;
}

#ifdef __cplusplus
}
#endif
