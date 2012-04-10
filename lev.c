#include "logpool_event.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <event2/thread.h>
#include <event2/buffer.h>
#include <event2/dns.h>
#include <event2/bufferevent.h>

static char *lev_set_protocol(char *p, uint16_t protocol, size_t klen, size_t vlen)
{
    struct logpool_protocol *buf = (struct logpool_protocol *) p;
    buf->protocol = protocol;
    buf->klen = (uint16_t) klen;
    buf->vlen = (uint16_t) vlen;
    return p + sizeof(struct logpool_protocol);
}

static void lev_callback(struct bufferevent *bev, short events, void *ptr)
{
    if (events & BEV_EVENT_CONNECTED) {
        fprintf(stderr, "Connect okay.\n");
    } else if (events & (BEV_EVENT_ERROR|BEV_EVENT_EOF)) {
        struct event_base *base = ptr;
        if (events & BEV_EVENT_ERROR) {
            int err = bufferevent_socket_get_dns_error(bev);
            if (err)
                fprintf(stderr, "DNS error: %s\n", evutil_gai_strerror(err));
        }
        bufferevent_free(bev);
        event_base_loopexit(base, NULL);
    }
}

static void lev_thread_init(struct lev *ev)
{
    struct event_base *base = event_base_new();
    struct evdns_base *dns_base;
    struct bufferevent *buff;
    buff = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
    bufferevent_setcb(buff, NULL, NULL, lev_callback, base);
    bufferevent_enable(buff, EV_READ|EV_WRITE);
    dns_base = evdns_base_new(base, 1);
    bufferevent_socket_connect_hostname(buff,
            dns_base, AF_INET, "127.0.0.1", 10000);
    ev->buff = buff;
}

static void *lev_thread_main(void *args)
{
    struct lev *ev = (struct lev *) args;
    assert(ev);
    lev_thread_init(ev);
    event_base_dispatch(bufferevent_get_base(ev->buff));
    return 0;
}

struct lev *lev_new(char *host, int port)
{
    struct lev *ev;
    static int once = 1;
    if (once) {
        once = 0;
        evthread_use_pthreads();
    }
    ev = malloc(sizeof(*ev));
    bzero(ev, sizeof(*ev));
    pthread_create(&ev->thread, NULL, lev_thread_main, ev);
    while (ev->buff == NULL) {
        asm volatile ("" ::: "memory");
    }
    return ev;
}

int lev_append(struct lev *ev, char *value, size_t vlen, uint32_t flags)
{
    if (bufferevent_write(ev->buff, value, vlen) != 0) {
        fprintf(stderr, "write error, v=('%s', %ld), flags=%x\n", value, vlen, flags);
        return 1;
    }
    return 0;
}

int lev_quit(struct lev *ev)
{
    struct logpool_protocol tmp;
    char *p = (char *) &tmp;
    lev_set_protocol(p, LOGPOOL_EVENT_QUIT, 0, 0);
    lev_append(ev, p, sizeof(tmp), 0);
    fprintf(stderr, "deinit\n");
    if (pthread_join(ev->thread, NULL) != 0) {
        fprintf(stderr, "pthread join failure.\n");
        abort();
    }
    return 0;
}

int lev_destory(struct lev *ev)
{
    lev_quit(ev);
    free(ev);
    return 0;
}
