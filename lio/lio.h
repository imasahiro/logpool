#include <stdint.h>
#include <stdio.h>
#include <pthread.h>

#ifndef LIO_H
#define LIO_H

#ifdef __cplusplus
extern "C" {
#endif

enum lio_status {
    LIO_OK = 0,
    LIO_FAILED = -1
};

enum LIO_MODE {
    LIO_MODE_READ   = (1 << 0),
    LIO_MODE_WRITE  = (1 << 1),
    LIO_MODE_CLIENT = (1 << 2),
    LIO_MODE_THREAD = (1 << 3)
};

#define LIO_BUFFER_SIZE 128
struct lio;
typedef int (*lio_cb)(struct lio *lio, const void *data, uint32_t nbyte);

struct query_list;
struct qengine;
struct chunk_stream;
struct bufferevent;

struct lio {
    lio_cb f_read;
    lio_cb f_write;
    uint32_t flags;
    uint32_t host;
    struct bufferevent *bev;
    struct query_list *q;
    struct qengine *engine;
    struct event_base *base;
    struct chunk_stream *cs; // use this at client
    pthread_t thread;
    int (*f_close)(struct lio *lio);
};

struct lio_api {
    const char *name;
    int (*f_init)(struct lio *lio, char *host, int port, int ev_mode);
    lio_cb f_read;
    lio_cb f_write;
    int (*f_close)(struct lio *lio);
};

extern struct lio *lio_open(char *host, int port, int mode, struct lio_api *api);
extern struct lio *lio_open_trace(char *host, int port);
extern int lio_close(struct lio *lio);
extern int lio_write(struct lio *lio, const void *data, uint32_t nbyte);
extern int lio_read(struct lio *lio, void *data, uint32_t nbyte);
extern int lio_sync(struct lio *lio);
extern int lio_dispatch(struct lio *lio);

#define LIO_DEBUG 1
#define LIO_DEBUG_LEVEL 0
#define debug_print(level, ...) do {\
    if (level >= LIO_DEBUG_LEVEL) {\
        if (LIO_DEBUG) {\
            fprintf(stderr, "[%s:%d] ", __func__, __LINE__);\
            fprintf(stderr, ## __VA_ARGS__);\
            fprintf(stderr, "\n");\
            fflush(stderr);\
        }\
    }\
} while (0)

#define mfence() asm volatile ("" ::: "memory")

#ifdef __cplusplus
}
#endif
#endif /* end of include guard */
