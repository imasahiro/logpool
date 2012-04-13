#include <cstdlib>
#include <event.h>
#include <event2/listener.h>
#include "logpool_event.h"

#ifndef LOGPOOLD_H_
#define LOGPOOLD_H_

#define BUFF_SIZE      1024*4
#define DEBUG 0
#define debug_print(...) do {\
    if (DEBUG) {\
        fprintf(stderr, ## __VA_ARGS__);\
    }\
} while (0)

namespace logpool {
struct lpevent {
    struct event_base *base;
    char buf[BUFF_SIZE];
    char *last_buf;
    int shift;
    lpevent(const char *server, int port) : last_buf(NULL), shift(0) {
        init(server, port);
    }
    bool init(const char *server, int port);
    void exec() {
        event_base_dispatch(base);
    }
    static void accept_cb(struct evconnlistener *lev,
            evutil_socket_t fd, struct sockaddr *sa, int socklen, void *ctx);
    static void read_cb(struct bufferevent *bev, void *ctx);
    static void event_cb(struct bufferevent *bev, short events, void *ctx);
};

struct log_data : public logpool_protocol {
    char data[128];
    bool process() {
        switch (protocol) {
        case LOGPOOL_EVENT_NULL:
        case LOGPOOL_EVENT_READ:
        default:
            /*TODO*/abort();
            break;
        case LOGPOOL_EVENT_WRITE:
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
private:
    lpevent *lp_;
    struct bufferevent *bev_;
    char *cur;
    int len;
public:
    explicit log_stream(lpevent *lp, struct bufferevent *bev) : lp_(lp), bev_(bev) {
        cur = (lp_->last_buf)?lp_->last_buf:lp_->buf;
        len = lp_->shift;
        lp_->shift = 0;
    }
    virtual ~log_stream() {
        lp_->shift = len;
        lp_->last_buf = cur;
    }
    log_data *get();
    bool empty() const;
private:
    bool reset(int request_size);
    int size() const;
    char *next(size_t offset);
};
} /* namespace logpool */

#endif /* end of include guard */
