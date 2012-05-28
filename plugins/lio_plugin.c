#include <stdlib.h>
#include <stdio.h>
#include "logpool.h"
#include "lpstring.h"
#include "logpool_internal.h"
#include "../lio/lio.h"
#include "../lio/protocol.h"
#include "../lio/util.h"

enum {
    LOGPOOL_FAILURE = -1,
    LOGPOOL_SUCCESS = 0
};

#define DEFAULT_SERVER "127.0.0.1"
#define DEFAULT_PORT   14801

#ifdef __cplusplus
extern "C" {
#endif

static struct lio *g_lio = NULL;
struct lio_plugin {
    char *buf;
    struct lio *lio;
    char base[1];
};

static uint16_t *emit_header(char *buf, int protocol, int logsize)
{
    struct Message *msg = ((struct Message*)buf);
    msg->crc32 = 0;
    msg->protocol = LOGPOOL_EVENT_WRITE;
    msg->logsize  = logsize;
    buf += LOG_PROTOCOL_SIZE;
    return (uint16_t *) buf;
}

static void *logpool_lio_init(logpool_t *logpool, logpool_param_t *p)
{
    struct lio_plugin *lp;
    lp = cast(struct lio_plugin *, logpool_string_init(logpool, p));
    lp->lio = g_lio;
    return cast(void *, lp);
}

static void logpool_lio_flush(logpool_t *logpool, void **args __UNUSED__)
{
    struct lio_plugin *lp = cast(struct lio_plugin *, logpool->connection);
    char *buf_orig = lp->buf;
    size_t size, bufsize;
    struct logfmt *fmt, *fmte;
    int ret;
    uint16_t *loginfo;

    size = logpool->logfmt_size + 1;
    fmt = cast(struct logpool *, logpool)->fmt;
    fmte = fmt + size - 1;
    loginfo = emit_header(lp->buf, LOGPOOL_EVENT_WRITE, size);
    lp->buf = (char *)(loginfo + size * 2);
    char *p = lp->buf;
    logpool->fn_key(logpool, logpool->logkey.v.u,
            logpool->logkey.k.seq, logpool->logkey.klen);
    *loginfo++ = strlen("TraceID");
    *loginfo++ = lp->buf - p - strlen("TraceID");
    assert(size > 0);

    while (fmt < fmte) {
        char *_buf = lp->buf;
        fmt->fn(logpool, fmt->k.key, fmt->v.u, fmt->klen, fmt->vlen);
        *loginfo++ = fmt->klen;
        *loginfo++ = (lp->buf - _buf) - fmt->klen;
        ++fmt;
    }
    cast(struct logpool *, logpool)->logfmt_size = 0;
    bufsize = (char*) lp->buf - buf_orig;
    ret = lio_write(lp->lio, buf_orig, bufsize);
    if (ret != LOGPOOL_SUCCESS) {
        /* TODO Error */
        fprintf(stderr, "Error!!\n");
        abort();
    }
    logpool_string_reset(logpool);
    ++(cast(struct logpool *, logpool)->logkey.k.seq);
}

static void logpool_lio_delim(logpool_t *ctx) {}
static void logpool_lio_string(logpool_t *ctx, const char *key, uint64_t v, short klen, short vlen)
{
    buffer_t *buf = cast(buffer_t *, ctx->connection);
    char *s = cast(char *, v);
    buf_put_string(buf, key, klen);
    buf_put_string(buf, s, vlen);
}

struct logapi TRACE_API = {
    logpool_string_null,
    logpool_string_bool,
    logpool_string_int,
    logpool_string_hex,
    logpool_string_float,
    logpool_string_char,
    logpool_lio_string,
    logpool_string_raw,
    logpool_lio_delim,
    logpool_lio_flush,
    logpool_lio_init,
    logpool_string_close,
    logpool_default_priority
};

extern struct keyapi *logpool_string_api_init(void);
struct keyapi *logpool_trace_api_init(void)
{
    char *serverinfo = getenv("LOGPOOL_SERVER");
    char host[128] = {0};
    int  port = DEFAULT_PORT;
    if (serverinfo) {
        char *pos;
        if ((pos = strchr(serverinfo, ':')) != NULL) {
            port = strtol(pos+1, NULL, 10);
            memcpy(host, serverinfo, pos - serverinfo);
        } else {
            memcpy(host, DEFAULT_SERVER, strlen(DEFAULT_SERVER));
        }
    } else {
        memcpy(host, DEFAULT_SERVER, strlen(DEFAULT_SERVER));
    }
    fprintf(stderr,"connect to [%s:%u]\n", host, port);
    g_lio = lio_open_trace(host, port);
    return logpool_string_api_init();
}

void logpool_trace_api_deinit(void)
{
    lio_close(g_lio);
}

int logpoold_start(char *host, int port)
{
    extern struct lio_api server_api;
    struct lio *lio = lio_open(host, port,
            LIO_MODE_READ|LIO_MODE_WRITE, &server_api);
    return lio_dispatch(lio);
}


static void *logpool_lio_client_init(logpool_t *logpool, logpool_param_t *p)
{
    extern struct lio_api client_api;
    struct lio_plugin *lp;
    struct logpool_param_trace *args = cast(struct logpool_param_trace *, p);
    char *host = (char*) args->host;
    long port = args->port;

    lp = cast(struct lio_plugin *, logpool_string_init(logpool, p));
    lp->lio = lio_open(host, port, LIO_MODE_READ|LIO_MODE_WRITE, &client_api);
    return cast(void *, lp);
}
static void logpool_lio_client_close(logpool_t *ctx)
{
    struct lio_plugin *lp = cast(struct lio_plugin *, ctx->connection);
    logpool_string_close(ctx);
    lio_close(lp->lio);
}

static struct logapi CLIENT_API = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    logpool_lio_client_init,
    logpool_lio_client_close,
    NULL
};

logpool_t *logpool_open_client(logpool_t *parent, char *host, int port)
{
    struct logpool_param_trace param = {8, 1024};
    logpool_init(0);
    param.host = host;
    param.port = port;
    return logpool_open(parent, &CLIENT_API, (struct logpool_param*) &param);
}

void logpool_query(logpool_t *logpool, char *q)
{
    struct lio_plugin *lp = cast(struct lio_plugin *, logpool->connection);
    char buf[128] = {};
    size_t len = emit_message(buf, LOGPOOL_EVENT_READ, 1,
            0, strlen(q), NULL, q);
    assert(lio_write(lp->lio, buf, len) == LIO_OK);
}

void *logpool_client_get(logpool_t *logpool, void *logbuf, size_t bufsize)
{
    struct lio_plugin *lp = cast(struct lio_plugin *, logpool->connection);
    if (lio_read(lp->lio, (char*) logbuf, bufsize) == LIO_FAILED)
        return NULL;
    return (void*) logbuf;
}

#ifdef __cplusplus
}
#endif
