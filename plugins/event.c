#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <event2/event.h>
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
#define DEFAULT_SERVER "127.0.0.1"
#define DEFAULT_PORT   10000

struct lev {
    struct bufferevent *buff;
    pthread_t thread;
    int join;
};

static struct lev g_event;

enum {
    LOGPOOL_FAILURE = -1,
    LOGPOOL_SUCCESS = 0
};

static int logpool_append(struct lev *ev, char *value, size_t vlen, uint32_t flags);

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
    lp->ev = &g_event;
    return cast(void *, lp);
}

static void logpool_LogPool_close(logctx_t *ctx)
{
    logpool_string_close(ctx);
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
    logpool_LogPool_close,
    logpool_default_priority
};

static int event_init(char *host, int port);
static int event_deinit(void);
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
    evthread_use_pthreads();
    event_init(host, port);
    return logpool_string_api_init();
}

void logpool_event_api_deinit(void)
{
    event_deinit();
}

static void eventcb(struct bufferevent *bev, short events, void *ptr)
{
#if 0
    fprintf(stderr, "ev: %d\n", (int)events);
#endif
    if (events & BEV_EVENT_CONNECTED) {
        fprintf(stderr,"Connect okay.\n");
    } else if (events & (BEV_EVENT_ERROR|BEV_EVENT_EOF)) {
        struct event_base *base = ptr;
        if (events & BEV_EVENT_ERROR) {
            int err = bufferevent_socket_get_dns_error(bev);
            if (err)
                fprintf(stderr,"DNS error: %s\n", evutil_gai_strerror(err));
        }
        /*fprintf(stderr,"Closing\n");*/
        bufferevent_free(bev);
        event_base_loopexit(base, NULL);
    }
}

static void ev_thread_init(struct lev *ev)
{
    struct event_base *base = event_base_new();
    struct evdns_base *dns_base;
    struct bufferevent *buff;
    buff = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
    bufferevent_setcb(buff, NULL, NULL, eventcb, base);
    bufferevent_enable(buff, EV_READ|EV_WRITE);
    dns_base = evdns_base_new(base, 1);
    bufferevent_socket_connect_hostname(
            buff, dns_base, AF_INET, "127.0.0.1", 10000);
    ev->buff = buff;
}

static void *event_main(void *args)
{
    struct lev *ev = (struct lev *) args;
    ev_thread_init(ev);
    event_base_dispatch(bufferevent_get_base(ev->buff));
    return 0;
}

static int logpool_append(struct lev *ev, char *value, size_t vlen, uint32_t flags)
{
    if (bufferevent_write(ev->buff, value, vlen) != 0) {
        fprintf(stderr, "write error, v=('%s', %ld), flags=%x\n", value, vlen, flags);
        return 1;
    }
    return 0;
}

static int event_init(char *host, int port)
{
    memset(&g_event, 0, sizeof(struct lev));
    pthread_create(&g_event.thread, NULL, event_main, &g_event);
    while (g_event.buff == NULL) {
        asm volatile ("" ::: "memory");
    }
    return 0;
}

static int event_deinit(void)
{
    struct logpool_protocol tmp;
    char *p = (char *) &tmp;
    bufferevent_lock(g_event.buff);
    logpool_write_protocol(p, LOGPOOL_EVENT_QUIT, 0, 0);
    bufferevent_setwatermark(g_event.buff, EV_WRITE|EV_WRITE, 128, 256);
    logpool_append(&g_event, p, sizeof(tmp), 0);
    bufferevent_unlock(g_event.buff);
    fprintf(stderr, "deinit\n");
    if (pthread_join(g_event.thread, NULL) != 0) {
        fprintf(stderr, "pthread join failure.\n");
        abort();
    }
    return 0;
}

#ifdef __cplusplus
}
#endif
