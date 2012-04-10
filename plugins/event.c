#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <event2/thread.h>
#include <event2/buffer.h>
#include <event2/dns.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>
#include "logpool.h"
#include "lpstring.h"
#include "logpool_internal.h"
#include "logpool_event.h"

#ifdef __cplusplus
extern "C" {
#endif

static int logpool_append(struct lev *ev, char *value, size_t vlen, uint32_t flags);
static struct lev *g_ev = NULL;

typedef struct lp {
    char *buf;
    struct lev *ev;
    char base[1];
} lp_t;


static void *logpool_LogPool_init(logctx_t *ctx, logpool_param_t *p)
{
    /*
     * struct logpool_param_logpool *args = cast(struct logpool_param_logpool *, p);
     * const char *host = args->host;
     * long port = args->port;
     */
    lp_t *lp = cast(lp_t *, logpool_string_init(ctx, p));
    lp->ev = g_ev;
    fprintf(stderr, "%p\n", lp->ev);
    return cast(void *, lp);
}

static char *logpool_write_protocol(char *p, uint16_t protocol, size_t klen, size_t vlen)
{
    struct logpool_protocol *buf = (struct logpool_protocol *) p;
    buf->protocol = protocol;
    buf->klen = (uint16_t) klen;
    buf->vlen = (uint16_t) vlen;
    return p + sizeof(struct logpool_protocol);
}

static void logpool_LogPool_flush(logctx_t *ctx, void **args __UNUSED__)
{
    lp_t *lp = cast(lp_t *, ctx->connection);
    char *buf_orig = lp->buf, *p;
    uint32_t flags = 0;
    struct logfmt *fmt = cast(struct logctx *, ctx)->fmt;
    size_t klen, vlen, i, size = ctx->logfmt_size;
    int ret;

    lp->buf = lp->buf + sizeof(struct logpool_protocol);
    p = ctx->fn_key(ctx, ctx->logkey.v.u, ctx->logkey.k.seq, ctx->logkey.siz);
    klen = p - (char*) buf_orig - sizeof(struct logpool_protocol);
    if (size) {
        void (*fn_delim)(logctx_t *) = ctx->formatter->fn_delim;
        fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
        ++fmt;
        for (i = 1; i < size; ++i, ++fmt) {
            fn_delim(ctx);
            fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
        }
        cast(struct logctx *, ctx)->logfmt_size = 0;
    }
    vlen = (char*) lp->buf - p;
    logpool_write_protocol(buf_orig, LOGPOOL_EVENT_WRITE, klen, vlen);
#if 0
    {
        static int __debug__ = 0;
        fprintf(stderr, "%d, '%s'\n", __debug__++, buf_orig+sizeof(struct logpool_protocol));
    }
#endif
    ret = logpool_append(lp->ev, buf_orig, sizeof(struct logpool_protocol)+klen+vlen, flags);
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

static int logpool_append(struct lev *ev, char *value, size_t vlen, uint32_t flags)
{
    return lev_append(ev, value, vlen, flags);
}

#ifdef __cplusplus
}
#endif
