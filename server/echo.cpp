#include <cstdio>
#include <cstring>
#include <arpa/inet.h>
#include <event.h>
#include <event2/listener.h>

#define SERVER_IP     "127.0.0.1"
#define SERVER_PORT    10000
#define BUFF_SIZE      1024

static void read_cb(struct bufferevent *bev, void *ctx) {
    char buff[BUFF_SIZE];
    int len;

    memset(buff, 0, sizeof(buff));

    /* Read data */
    len = bufferevent_read(bev, buff, sizeof(buff));
    fprintf(stderr,"read: len=[%d] data=[%s]\n", len, buff);
}


static void event_cb(struct bufferevent *bev, short events, void *ctx) {
    if (events & BEV_EVENT_EOF) {
        fprintf(stderr,"client disconnect\n");
        bufferevent_free(bev);
    } else if (events & BEV_EVENT_TIMEOUT) {
        fprintf(stderr,"client timeout\n");
        bufferevent_free(bev);
    } else {
        /* Other case, maybe error occur */
        bufferevent_free(bev);
    }
}
struct lpevent {
    struct event_base *base;
    lpevent(const char *server, int port) {
        init(server, port);
    }
    bool init(const char *server, int port);
    void exec() {
        /* Enter event loop */
        event_base_dispatch(base);
    }
    static void accept_cb(struct evconnlistener *lev,
            evutil_socket_t fd, struct sockaddr *sa, int socklen, void *ctx);
};

bool lpevent::init(const char *server, int port) {
    struct sockaddr_in sin;
    /* Set up server address */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(server);
    sin.sin_port = htons(port);

    /* Initialize event base */
    if ((base = event_base_new()) == NULL) {
        fprintf(stderr,"event_base_new() failed\n");
        return false;
    }

    /* Bind socket */
    struct evconnlistener *lev;
    lev = evconnlistener_new_bind(base, accept_cb, this,
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
            (struct sockaddr *) &sin, sizeof(sin));

    if (lev == NULL) {
        fprintf(stderr,"bind() failed\n");
        return -1;
    } else {
        fprintf(stderr,"bind to [%s:%u] successfully\n", SERVER_IP, SERVER_PORT);
    }
    return true;
}

void lpevent::accept_cb(struct evconnlistener *lev, evutil_socket_t fd,
        struct sockaddr *sa, int socklen, void *ctx) {
    lpevent *lp = reinterpret_cast<lpevent *>(ctx);
    struct event_base *evbase = lp->base;
    struct bufferevent *bev;

    fprintf(stderr,"client connect from [%s:%u] over fd [%d]\n",
            inet_ntoa(((struct sockaddr_in *) sa)->sin_addr),
            (unsigned short) ntohs(((struct sockaddr_in *) sa)->sin_port), fd);

    /* Create socket-based buffer event */
    bev = bufferevent_socket_new(evbase, fd, BEV_OPT_CLOSE_ON_FREE);
    if (bev == NULL) {
        fprintf(stderr,"bufferevent_socket_new() failed\n");
        evutil_closesocket(fd);
        return;
    }

    /* Set up callback function */
    bufferevent_setcb(bev, read_cb, NULL, event_cb, NULL);

    /* Set up timeout for data read */
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    bufferevent_set_timeouts(bev, &tv, NULL);

    /* Enable read event */
    bufferevent_enable(bev, EV_READ);
}


int main(int argc, char const* argv[])
{
    lpevent ev(SERVER_IP, SERVER_PORT);
    ev.exec();
    return 0;
}
