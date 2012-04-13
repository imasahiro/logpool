#include <stdlib.h>
#include <stdio.h>
#include "logpool.h"
#include "lpstring.h"
#include "logpool_internal.h"
#include "logpool_event.h"

#define DEFAULT_SERVER "127.0.0.1"
#define DEFAULT_PORT   10000

#ifdef __cplusplus
extern "C" {
#endif

static struct lev *g_ev = NULL;

typedef struct lp {
    char *buf;
    struct lev *ev;
    char base[1];
} lp_t;

static void *logpool_LogPool_init(logctx_t *ctx, logpool_param_t *p)
{
    lp_t *lp = cast(lp_t *, logpool_string_init(ctx, p));
    lp->ev = g_ev;
    return cast(void *, lp);
}

static void logpool_LogPool_flush(logctx_t *ctx, void **args __UNUSED__)
{
    lp_t *lp = cast(lp_t *, ctx->connection);
    char *buf_orig = lp->buf, *p;
    size_t klen, vlen, size;
    struct logfmt *fmt, *fmte;
    int ret;

    size = ctx->logfmt_size;
    fmt = cast(struct logctx *, ctx)->fmt;
    fmte = fmt + size;
    lp->buf = lp->buf + sizeof(struct logpool_protocol);
    p = ctx->fn_key(ctx, ctx->logkey.v.u, ctx->logkey.k.seq, ctx->logkey.siz);
    klen = p - (char*) buf_orig - sizeof(struct logpool_protocol);
    if (size) {
        void (*fn_delim)(logctx_t *) = ctx->formatter->fn_delim;
        fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
        ++fmt;
        while (fmt < fmte) {
            fn_delim(ctx);
            fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
            ++fmt;
        }
        cast(struct logctx *, ctx)->logfmt_size = 0;
    }
    vlen = (char*) lp->buf - p;
    ret = lev_write(lp->ev, buf_orig, klen, vlen, 0);
    if (ret != LOGPOOL_SUCCESS) {
        /* TODO Error */
        fprintf(stderr, "Error!!\n");
        abort();
    }
    logpool_string_reset(ctx);
    ++(cast(struct logctx *, ctx)->logkey.k.seq);
}

struct logapi EVENT_API = {
    logpool_string_null,
    logpool_string_bool,
    logpool_string_int,
    logpool_string_hex,
    logpool_string_float,
    logpool_string_char,
    logpool_string_string,
    logpool_string_raw,
    logpool_string_delim,
    logpool_LogPool_flush,
    logpool_LogPool_init,
    logpool_string_close,
    logpool_default_priority
};

extern struct keyapi *logpool_string_api_init(void);
struct keyapi *logpool_event_api_init(void)
{
    char *serverinfo = getenv("LOGPOOL_SERVER");
    char host[128] = {0};
    int  port = DEFAULT_PORT;
    if (serverinfo) {
        char *pos;
        if ((pos = strchr(serverinfo, ':')) != NULL) {
            port = strtol(pos+1, NULL, 10);
            memcpy(host, serverinfo, pos - serverinfo);
        } else {
            memcpy(host, DEFAULT_SERVER, strlen(DEFAULT_SERVER));
        }
    }
    fprintf(stderr,"connect to [%s:%u]\n", host, port);
    g_ev = lev_new(host, port);
    return logpool_string_api_init();
}

void logpool_event_api_deinit(void)
{
    lev_destory(g_ev);
}

#ifdef __cplusplus
}
#endif
