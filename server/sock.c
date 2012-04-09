#include <string.h>
#include <pthread.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/dns.h>
#include <event2/bufferevent.h>

void readcb(struct bufferevent *bev, void *ptr)
{
    char buf[16];
    int n;
    while ((n = bufferevent_read(bev, buf, sizeof(buf))) > 0) {
        char buf2[16] = {0};
        memcpy(buf2, buf, n);
        fprintf(stderr, "write::size=%d, '%s'\n", n, buf2);
    }
    fprintf(stderr, "%s:%d bev=%p\n", __func__, __LINE__, bev);
    fprintf(stderr, "write end\n");
}

void eventcb(struct bufferevent *bev, short events, void *ptr)
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

struct lev {
    struct event_base *base;
    struct bufferevent *buff;
};

void ev_init(struct lev *ev)
{
    struct evdns_base *dns_base;
    ev->base = event_base_new();
    ev->buff = bufferevent_socket_new(ev->base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(ev->buff, NULL, readcb, eventcb, ev->base);
    bufferevent_enable(ev->buff, EV_READ|EV_WRITE);
    dns_base = evdns_base_new(ev->base, 1);
    bufferevent_socket_connect_hostname(
            ev->buff, dns_base, AF_INET, "127.0.0.1", 10000);
}

void *event_main(void *args)
{
    struct lev *ev = (struct lev *) args;
    ev_init(ev);
    fprintf(stderr, "thread start\n");
    const char text[] = "123456789012345hello";
    bufferevent_write(ev->buff, text, strlen(text));
    event_base_dispatch(ev->base);
    fprintf(stderr, "thread finish\n");
    return 0;
}

int main(int argc, const char *argv[])
{
    struct lev ev;
    pthread_t thread;
    pthread_create(&thread, NULL, event_main, &ev);
    while (ev.buff == NULL) {}

    int i;
    for (i = 0; i < 10; ++i) {
        const char text[] = "123456789012345";
        if (bufferevent_write(ev.buff, text, strlen(text)) != 0) {
            fprintf(stderr, "write error\n");
        }
    }

    fprintf(stderr, "flush\n");
    if (bufferevent_flush(ev.buff, EV_WRITE, BEV_EVENT_EOF) == -1) {
        fprintf(stderr, "flush failure\n");
    }
    pthread_join(thread, NULL);
    return 0;
}

