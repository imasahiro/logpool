#include <cstdio>
#include <cstring>
#include <cassert>
#include <arpa/inet.h>
#include "logpoold.h"

namespace logpool {

bool log_stream::reset(int request_size)
{
    int old_len = len;
    if (len) {
        memmove(lp_->buf, cur, len);
    }
    len += bufferevent_read(bev_, lp_->buf + len, BUFF_SIZE - len);
    cur  = lp_->buf;
    debug_print("reset %d=>%d\n", old_len, len);
    return len >= request_size;
}
int log_stream::size() const {
    return evbuffer_get_length(bufferevent_get_input(bev_));
}

bool log_stream::empty() const {
    debug_print("empty %d, %d, %d\n", len, size(), len >= 0);
    assert(len >= 0);
    return len == 0 && size() == 0;
}
char *log_stream::next(size_t offset) {
    char *d = cur;
    offset += sizeof(logpool_protocol);
    debug_print("next offset=%lu, old_len=%d\n", offset, len);
    len -= offset;
    assert(len >= 0);
    cur += offset;
    return d;
}
log_data *log_stream::get() {
    if (len < (int) sizeof(logpool_protocol)) {
        debug_print("fetch protocol\n");
        if (size() <= 0) {
            debug_print("fetch protocol failed\n");
            return NULL;
        }
        if (!reset(sizeof(logpool_protocol))) {
            return NULL;
        }
    }
    debug_print("len=%d\n", len);
    while (len > 0) {
        log_data *d = (log_data *) cur;
        uint16_t klen, vlen;
        int reqsize;
        klen = d->klen;
        vlen = d->vlen;
        reqsize = (int)(sizeof(logpool_protocol) + klen + vlen);
        debug_print("%d, %d, check=%d\n", len, klen+vlen, len < reqsize);
        if (len < reqsize) {
            debug_print("fetch body\n");
            if (size() <= 0) {
                debug_print("fetch body failed\n");
                return NULL;
            }
            if (!reset(reqsize)) {
                return NULL;
            }
        }
        return (log_data*) next(klen + vlen);
    }
    return NULL;
}

void lpevent::read_cb(struct bufferevent *bev, void *ctx)
{
    lpevent *lp = reinterpret_cast<lpevent *>(ctx);
    log_stream logs(lp, bev);
    debug_print("read_cb\n");
    while (!logs.empty()) {
        debug_print("!empty\n");
        log_data *log = logs.get();
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
        fprintf(stderr,"bind to [%s:%u] successfully\n", server, port);
    }
    return true;
}

void lpevent::accept_cb(struct evconnlistener *lev, evutil_socket_t fd,
        struct sockaddr *sa, int socklen, void *ctx)
{
    (void)lev;(void)socklen;
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
}
