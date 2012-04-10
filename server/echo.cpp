#include <cstdio>
#include <cstring>
#include <cassert>
#include <arpa/inet.h>
#include <event.h>
#include <event2/listener.h>
#include "../logpool_event.h"

#define SERVER_IP     "127.0.0.1"
#define SERVER_PORT    10000
#define BUFF_SIZE      1024*4

struct lpevent {
    struct event_base *base;
    char buf[BUFF_SIZE];
    char *last_buf;
    int shift;
    lpevent(const char *server, int port) : last_buf(NULL) {
        init(server, port);
    }
    bool init(const char *server, int port);
    void exec() {
        /* Enter event loop */
        event_base_dispatch(base);
    }
    static void accept_cb(struct evconnlistener *lev,
            evutil_socket_t fd, struct sockaddr *sa, int socklen, void *ctx);
    static void read_cb(struct bufferevent *bev, void *ctx);
    static void event_cb(struct bufferevent *bev, short events, void *ctx);
};
#define DEBUG 0

#define debug_print(fmt, ...) do {\
    if (DEBUG) {\
        fprintf(stderr, fmt, # __VA_ARGS__);\
    }\
} while (0)

struct logpool_event_data {
    struct logpool_protocol base;
    char data[];
};

int total_log_count = 0;

struct log_t : public logpool_protocol {
    char data[128];
    bool process() {
        switch (protocol) {
        case LOGPOOL_EVENT_NULL:
        case LOGPOOL_EVENT_READ:
        default:
            /*TODO*/abort();
            break;
        case LOGPOOL_EVENT_WRITE:
            ++total_log_count;
            debug_print("%d, %d, '%s'\n", klen, vlen, data);
            break;
        case LOGPOOL_EVENT_QUIT:
            debug_print("quit, %d, %d\n", klen, vlen);
            return false;
            break;
        }
        return true;
    }
};

struct log_stream {
    lpevent *lp_;
    struct bufferevent *bev_;
    char *cur;
    int len;
    explicit log_stream(lpevent *lp, struct bufferevent *bev) : lp_(lp), bev_(bev) {
        cur = (lp_->last_buf)?lp_->last_buf:lp_->buf;
        len = lp_->shift;
        lp_->shift = 0;
    }
    ~log_stream() {
        lp_->shift = len;
        lp_->last_buf = cur;

    }
    void debug(int len) {
#if 0
        if (len >= (int)sizeof(logpool_protocol)) {
            debug_print("0:%p, '%s', cur=%p, '%s'\n",
                    lp_->buf, lp_->buf+sizeof(logpool_protocol),
                    cur, cur+sizeof(logpool_protocol));
        }
#endif
    }

    void reset() {
        int old_len = len;
        debug(old_len);
        if (len) {
            memmove(lp_->buf, cur, len);
        }
        len += bufferevent_read(bev_, lp_->buf + len, BUFF_SIZE - len);
        debug(old_len);
        cur  = lp_->buf;
        debug_print("reset %d=>%d\n", old_len, len);
    }
    int size() const {
        return evbuffer_get_length(bufferevent_get_input(bev_));
    }
    bool empty() const {
        debug_print("empty %d, %d\n", len, size());
        return len == 0 && size() == 0;
    }
    char *next(size_t offset) {
        char *d = cur;
        offset += sizeof(logpool_protocol);
        len -= offset;
        cur += offset;
        return d;
    }
    log_t *get() {
        if (len < (int) sizeof(logpool_protocol)) {
            debug_print("fetch protocol\n");
            if (size() <= 0) {
                debug_print("fetch protocol failed\n");
                return NULL;
            }
            reset();
        }
        debug_print("len=%d\n", len);
        while (len > 0) {
            log_t *d = (log_t *) cur;
            uint16_t klen, vlen;
            klen = d->klen;
            vlen = d->vlen;
            if (len < (int)(sizeof(logpool_protocol) + klen + vlen)) {
                debug_print("fetch body\n");
                if (size() <= 0) {
                    debug_print("fetch body failed\n");
                    return NULL;
                }
                reset();
            }
            return (log_t*) next(klen + vlen);
        }
        return NULL;
    }
};

void lpevent::read_cb(struct bufferevent *bev, void *ctx)
{
    lpevent *lp = reinterpret_cast<lpevent *>(ctx);
    log_stream logs(lp, bev);
    debug_print("read_cb\n");
    while (!logs.empty()) {
        debug_print("!empty\n");
        log_t *log = logs.get();
        if (!log) {
            break;
        }
        if (log->process() == false) {
            bufferevent_free(bev);
            break;
        }
    }
}

void lpevent::event_cb(struct bufferevent *bev, short events, void *ctx)
{
    if (events & BEV_EVENT_EOF) {
        fprintf(stderr,"client disconnect\n");
        bufferevent_free(bev);
    } else if (events & BEV_EVENT_TIMEOUT) {
        lpevent *lp = reinterpret_cast<lpevent *>(ctx);
        lp->shift = 0;
        fprintf(stderr,"client timeout\n");
        bufferevent_free(bev);
    } else {
        /* Other case, maybe error occur */
        bufferevent_free(bev);
    }
}

bool lpevent::init(const char *server, int port)
{
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
        struct sockaddr *sa, int socklen, void *ctx)
{
    lpevent *lp = reinterpret_cast<lpevent *>(ctx);
    struct event_base *evbase = lp->base;
    struct bufferevent *bev;

    fprintf(stderr,"client connect from [%s:%u] over fd [%d]\n",
            inet_ntoa(((struct sockaddr_in *) sa)->sin_addr),
            (unsigned short) ntohs(((struct sockaddr_in *) sa)->sin_port), fd);

    bev = bufferevent_socket_new(evbase, fd, BEV_OPT_CLOSE_ON_FREE);
    if (bev == NULL) {
        fprintf(stderr,"bufferevent_socket_new() failed\n");
        evutil_closesocket(fd);
        return;
    }

    bufferevent_setcb(bev, read_cb, NULL, event_cb, ctx);

    /* Set up timeout for data read */
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    bufferevent_set_timeouts(bev, &tv, NULL);
    bufferevent_enable(bev, EV_READ);
    bufferevent_disable(bev, EV_WRITE);
    //bufferevent_setwatermark(bev, EV_READ, BUFF_SIZE/2, BUFF_SIZE);
}

int main(int argc, char const* argv[])
{
    lpevent ev(SERVER_IP, SERVER_PORT);
    ev.exec();
    return 0;
}
