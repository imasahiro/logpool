#include "lio.h"
#include "stream.h"
#include "query.h"
#include <assert.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

#ifdef __cplusplus
extern "C" {
#endif

static void server_event_callback(struct bufferevent *bev, short events, void *ctx)
{
    (void)ctx;
    if (events & BEV_EVENT_EOF) {
        debug_print(1, "client disconnect");
        bufferevent_free(bev);
    } else if (events & BEV_EVENT_TIMEOUT) {
        struct lio *lio = (struct lio *) ctx;
        debug_print(1, "client timeout e=%p, events=%x", bev, events);
        query_delete_connection(lio->q, bev);
        bufferevent_free(bev);
    } else {
        /* Other case, maybe error occur */
        bufferevent_free(bev);
    }
}

static void server_read_callback(struct bufferevent *bev, void *ctx)
{
    struct lio *lio = (struct lio *) ctx;
    debug_print(0, "read_cb bev=%p", bev);
    struct chunk_stream stream, *cs = chunk_stream_init(&stream, lio, bev);
    while (!chunk_stream_empty(cs)) {
        int log_size;
        struct log_data *log = chunk_stream_get(cs, &log_size);
        if (log == NULL) {
            break;
        }
        char *data = log_get_data((struct Log *) log);
        debug_print(0, "%d %d %s", log->protocol, log->logsize, data);
        switch (log_data_protocol(log)) {
        case LOGPOOL_EVENT_READ:
            debug_print(1, "R %d %d, '%s'", log->klen, log->vlen, data);
            query_add(lio->engine, (struct Query*) log, bev, lio->q);
            break;
        case LOGPOOL_EVENT_WRITE:
            debug_print(1, "W %d, %d, '%s'", log->klen, log->vlen, data);
            query_exec((struct Log *) log, log_size, lio->q);
            break;
        case LOGPOOL_EVENT_QUIT:
            debug_print(1, "Q %d, %d\n", log->klen, log->vlen);
            query_delete_connection(lio->q, bev);
            bufferevent_free(bev);
            goto L_exit;
        case LOGPOOL_EVENT_NULL:
        default:
            /*TODO*/abort();
            break;
        }
    }
    L_exit:;
    chunk_stream_deinit(cs);
    return;
}

static void server_write_callback(struct bufferevent *bev, void *ctx)
{
    debug_print(0, "write_cb bev=%p", bev);
}

static void server_accept_callback(struct evconnlistener *lev, evutil_socket_t fd,
        struct sockaddr *sa, int socklen, void *ctx)
{
    (void)socklen;
    struct event_base *base = evconnlistener_get_base(lev);
    struct bufferevent *bev;

    debug_print(1, "client connect from [%s:%u] over fd [%d]",
            inet_ntoa(((struct sockaddr_in *) sa)->sin_addr),
            (unsigned short) ntohs(((struct sockaddr_in *) sa)->sin_port), fd);

    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (bev == NULL) {
        debug_print(0, "bufferevent_socket_new() failed");
        evutil_closesocket(fd);
        return;
    }

    bufferevent_setcb(bev, server_read_callback,
            server_write_callback, server_event_callback, ctx);

    bufferevent_enable(bev, EV_READ|EV_WRITE);

    //FIXME
    //struct timeval tv;
    //tv.tv_sec = 10;
    //tv.tv_usec = 0;
    //bufferevent_set_timeouts(bev, &tv, &tv);
}

static int lio_server_init(struct lio *lio, char *host, int port, int ev_mode)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(host);
    sin.sin_port = htons(port);

    struct evconnlistener *lev;
    struct event_base *base = event_base_new();
    lev = evconnlistener_new_bind(base, server_accept_callback, lio,
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
            (struct sockaddr *) &sin, sizeof(sin));
    if (lev == NULL) {
        debug_print(9, "bind() failed");
        return LIO_FAILED;
    }
    lio->base = base;
    lio->q = query_new();
    lio->engine = qengine_init();
    return LIO_OK;
}

static int lio_server_write(struct lio *lio, const void *data, uint32_t nbyte)
{
    if (bufferevent_write(lio->bev, data, nbyte) != 0) {
        fprintf(stderr, "write error, v=('%p', %u)\n", data, nbyte);
        return LIO_FAILED;
    }
    return LIO_OK;
}

static int lio_server_read(struct lio *lio, const void *data, uint32_t nbyte)
{
    (void)lio;(void)data;(void)nbyte;
    debug_print(0, "read");
    return LIO_FAILED;
}

static int lio_server_close(struct lio *lio)
{
    qengine_exit(lio->engine);
    query_delete(lio->q);
    return LIO_OK;
}

struct lio_api server_api = {
    "server",
    lio_server_init,
    lio_server_read,
    lio_server_write,
    lio_server_close
};

#ifdef __cplusplus
}
#endif
