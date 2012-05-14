#include <stdint.h>
#include <pthread.h>
#include <event2/event.h>
#ifndef LOGPOOL_EVENT_H_
#define LOGPOOL_EVENT_H_

enum LOGPOOL_EVENT_PROTOCOL {
    LOGPOOL_EVENT_NULL,
    LOGPOOL_EVENT_WRITE,
    LOGPOOL_EVENT_READ,
    LOGPOOL_EVENT_QUIT
};

struct logpool_protocol {
    uint16_t protocol, klen;
    uint16_t vlen;
};

struct lev {
    struct bufferevent *buff;
    pthread_t thread;
};

struct lev *lev_new(char *host, int port);
int lev_write(struct lev *ev, char *packed_data, size_t klen, size_t vlen, uint32_t flags);
int lev_append(struct lev *ev, char *value, size_t vlen, uint32_t flags);
int lev_quit(struct lev *ev);
int lev_destory(struct lev *ev);

enum {
    LOGPOOL_FAILURE = -1,
    LOGPOOL_SUCCESS = 0
};


#endif /* end of include guard */
