#include "lio.h"
#include "stream.h"
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <event2/event.h>
#include <event2/dns.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#ifdef __cplusplus
extern "C" {
#endif

static void client_cb_event(struct bufferevent *bev, short events, void *ctx)
{
    struct event_base *base = ctx;
    if (events & BEV_EVENT_CONNECTED) {
        debug_print(0, "Connect okay.");
    } else if (events & BEV_EVENT_TIMEOUT) {
        debug_print(0, "server timeout");
        bufferevent_free(bev);
        event_base_loopexit(base, NULL);
    } else if (events & (BEV_EVENT_ERROR|BEV_EVENT_EOF)) {
        if (events & BEV_EVENT_ERROR) {
            int err = bufferevent_socket_get_dns_error(bev);
            if (err)
                fprintf(stderr, "DNS error: %s\n", evutil_gai_strerror(err));
        }
        bufferevent_free(bev);
        event_base_loopexit(base, NULL);
    }
}

static void client_cb_read(struct bufferevent *bev, void *ctx)
{
    debug_print(0, "read_cb");
}

static void client_cb_write(struct bufferevent *bev, void *ctx)
{
    debug_print(0, "write_cb");
}

static void lio_thread_start(struct lio *lio);
static int lio_client_init(struct lio *lio, char *host, int port, int ev_mode)
{
    struct event_base *base = event_base_new();
    struct evdns_base *dns_base;
    struct bufferevent *bev;
    bev = bufferevent_socket_new(base, -1,
            BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
    bufferevent_setcb(bev, client_cb_read,
            client_cb_write, client_cb_event, base);

    bufferevent_enable(bev, ev_mode);
    dns_base = evdns_base_new(base, 1);
    int ret = bufferevent_socket_connect_hostname(bev,
            dns_base, AF_INET, host, port);
    if (ret == -1) {
        bufferevent_free(bev);
        lio->bev = NULL;
        return LIO_FAILED;
    }

    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    bufferevent_set_timeouts(bev, NULL, &tv);

    lio->bev = bev;
    lio_thread_start(lio);
    return LIO_OK;
}

static void *lio_thread_main(void *args)
{
    struct lio *lio = (struct lio *) args;
    assert(lio);
    fprintf(stderr, "%s start\n", __func__);
    lio->flags |= LIO_MODE_THREAD;
    mfence();
    event_base_dispatch(bufferevent_get_base(lio->bev));
    lio->flags ^= LIO_MODE_THREAD;
    fprintf(stderr, "%s exit\n", __func__);
    mfence();
    return 0;
}

static void lio_thread_start(struct lio *lio)
{
    pthread_create(&lio->thread, NULL, lio_thread_main, lio);
    while (lio->flags & LIO_MODE_THREAD) {
        mfence();
    }
}

static int lio_client_write(struct lio *lio, const void *data, uint32_t nbyte)
{
    if (bufferevent_write(lio->bev, data, nbyte) != 0) {
        fprintf(stderr, "write error, v=('%p', %u)\n", data, nbyte);
        return LIO_FAILED;
    }
    return LIO_OK;
}

static int lio_client_read(struct lio *lio, const void *data, uint32_t nbyte)
{
    struct chunk_stream *cs;
    if (lio->cs == NULL) {
        cs = chunk_stream_new(lio, lio->bev);
        lio->cs = cs;
    } else {
        cs = lio->cs;
    }
    if (lio->bev) {
        int log_size;
        struct Log *log;
        L_redo:;
        while (chunk_stream_empty(cs)) {
            usleep(1);
        }
        log = chunk_stream_get(cs, &log_size);
        if (!log) {
            fprintf(stderr, "log is null\n");
            goto L_redo;
        }
        if (log_data_process(log) == LOGPOOL_EVENT_QUIT) {
            bufferevent_free(lio->bev);
            debug_print(1, "stream connection close");
            return LIO_FAILED;
        }
        memcpy((void*)data, log, nbyte);
        return LIO_OK;
    }
    debug_print(1, "stream was not connected");
    return LIO_FAILED;
}

static int lio_client_close(struct lio *lio)
{
    if (lio->flags & LIO_MODE_THREAD) {
        if (pthread_join(lio->thread, NULL) != 0) {
            fprintf(stderr, "pthread join failure. %s\n", strerror(errno));
            abort();
            return LIO_FAILED;
        }
    }
    return LIO_OK;
}

struct lio_api client_api = {
    "client",
    lio_client_init,
    lio_client_read,
    lio_client_write,
    lio_client_close
};

#ifdef __cplusplus
}
#endif
