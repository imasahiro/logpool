#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/dns.h>
#include <event2/bufferevent.h>
#include "logpool.h"
#include "lpstring.h"
#include "logpool_internal.h"

#ifdef __cplusplus
extern "C" {
#endif
#define DEFAULT_SERVER "127.0.0.1"
#define DEFAULT_PORT   10000

struct lev {
    struct bufferevent *buff;
    pthread_t thread;
};

enum {
    LOGPOOL_FAILURE = -1,
    LOGPOOL_SUCCESS = 0
};
static int logpool_append(struct lev *ev, char *key, size_t klen, char *value, size_t vlen, uint32_t flags);

typedef struct lp {
    char *buf;
    struct lev *ev;
    char base[1];
} lp_t;

static void *logpool_LogPool_init(logctx_t *ctx, logpool_param_t *p)
{
    struct logpool_param_logpool *args = cast(struct logpool_param_logpool *, p);
    const char *host = args->host;
    long port = args->port;
    lp_t *lp = cast(lp_t *, logpool_string_init(ctx, p));
    (void)host;(void)port;
    lp->ev = NULL;
    return cast(void *, lp);
}

static void logpool_LogPool_close(logctx_t *ctx)
{
    lp_t *lp = cast(lp_t *, ctx->connection);
    (void)lp;
    /*memcached_free(lp->ev);*/
    logpool_string_close(ctx);
}

static void logpool_LogPool_flush(logctx_t *ctx, void **args __UNUSED__)
{
    lp_t *lp = cast(lp_t *, ctx->connection);
    char key[128] = {0}, *buf_orig = lp->buf, *p;
    char *value = lp->base;
    uint32_t flags = 0;
    size_t klen, vlen;
    struct logfmt *fmt = cast(struct logctx *, ctx)->fmt;
    size_t i, size = ctx->logfmt_size;
    int ret;

    lp->buf = key;
    p = ctx->fn_key(ctx, ctx->logkey.v.u, ctx->logkey.k.seq, ctx->logkey.siz);
    klen = p - (char*) key;
    lp->buf = buf_orig;

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

    vlen = (char*) lp->buf - value;
    ret = logpool_append(lp->ev, key, klen, value, vlen, flags);
    if (ret != LOGPOOL_SUCCESS) {
        /* TODO Error */
        fprintf(stderr, "Error!!\n");
        abort();
    }
    logpool_string_reset(ctx);
    ++(cast(struct logctx *, ctx)->logkey.k.seq);
}

struct logapi LOGPOOL_API = {
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
    logpool_LogPool_close,
    logpool_default_priority
};

static int event_init(char *host, int port);
static int event_deinit(void);
extern struct keyapi *logpool_string_api_init(void);
struct keyapi *logpool_event_api_init(void)
{
    /* init threads */
    char *serverinfo = getenv("LOGPOOL_SERVER");
    char host[128] = {0};
    int  port;
    if (serverinfo) {
        char *pos;
        if ((pos = strchr(serverinfo, ':')) != NULL) {
            port = strtol(pos+1, NULL, 10);
            memcpy(host, serverinfo, pos - serverinfo);
            fprintf(stderr, "%s&%d\n", host, port);
        } else {
            memcpy(host, DEFAULT_SERVER, strlen(DEFAULT_SERVER));
            port = DEFAULT_PORT;
        }
    }
    event_init(host, port);
    return logpool_string_api_init();
}

void logpool_event_api_deinit(void)
{
    event_deinit();
}

static void eventcb(struct bufferevent *bev, short events, void *ptr)
{
    fprintf(stderr, "ev: %d\n", (int)events);
    if (events & BEV_EVENT_CONNECTED) {
        fprintf(stderr,"Connect okay.\n");
    } else if (events & (BEV_EVENT_ERROR|BEV_EVENT_EOF)) {
        struct event_base *base = ptr;
        if (events & BEV_EVENT_ERROR) {
            int err = bufferevent_socket_get_dns_error(bev);
            if (err)
                fprintf(stderr,"DNS error: %s\n", evutil_gai_strerror(err));
        }
        fprintf(stderr,"Closing\n");
        bufferevent_free(bev);
        event_base_loopexit(base, NULL);
    }
}

static void ev_thread_init(struct lev *ev)
{
    struct event_base *base = event_base_new();
    struct evdns_base *dns_base;
    ev->buff = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(ev->buff, NULL, NULL, eventcb, base);
    bufferevent_enable(ev->buff, EV_READ|EV_WRITE);
    dns_base = evdns_base_new(base, 1);
    bufferevent_socket_connect_hostname(
            ev->buff, dns_base, AF_INET, "127.0.0.1", 10000);
}

static void *event_main(void *args)
{
    struct lev *ev = (struct lev *) args;
    ev_thread_init(ev);
    fprintf(stderr, "thread start\n");
    event_base_dispatch(bufferevent_get_base(ev->buff));
    fprintf(stderr, "thread finish\n");
    return 0;
}

static struct lev g_event;
static int logpool_append(struct lev *ev, char *key, size_t klen, char *value, size_t vlen, uint32_t flags)
{
    if (bufferevent_write(ev->buff, value, vlen) != 0) {
        fprintf(stderr, "write error, k=('%s', %ld), v=('%s', %ld), flags=%x\n",
                key, klen, value, vlen, flags);
        return 1;
    }
    return 0;
}

static int event_init(char *host, int port)
{
    memset(&g_event, 0, sizeof(struct lev));
    pthread_create(&g_event.thread, NULL, event_main, &g_event);
    do {} while (g_event.buff == NULL);
    return 0;
}

static int event_deinit(void)
{
    pthread_join(g_event.thread, NULL);
    return 0;
}

#ifdef __cplusplus
}
#endif
