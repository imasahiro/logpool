#include <string.h>
#include <pthread.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/dns.h>
#include <event2/bufferevent.h>

#define EV_BUFSIZE 128

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

struct lev {
    struct bufferevent *buff;
};

void ev_init(struct lev *ev)
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

void *event_main(void *args)
{
    struct lev *ev = (struct lev *) args;
    ev_init(ev);
    fprintf(stderr, "thread start\n");
    const char text[] = "123456789012345hello";
    bufferevent_write(ev->buff, text, strlen(text));
    event_base_dispatch(bufferevent_get_base(ev->buff));
    fprintf(stderr, "thread finish\n");
    return 0;
}

int main(int argc, const char *argv[])
{
    struct lev ev = {};
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
    pthread_join(thread, NULL);
    return 0;
}

