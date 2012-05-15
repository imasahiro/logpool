#include <stdlib.h>
#include <stdio.h>
#include "logpool.h"
#include "lpstring.h"
#include "logpool_internal.h"
#include "../lio/lio.h"
#include "../lio/protocol.h"

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

static void *logpool_lio_init(logpool_t *logpool, logpool_param_t *p)
{
    struct lio_plugin *lp = cast(struct lio_plugin *, logpool_string_init(logpool, p));
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
    loginfo = ((uint16_t*)lp->buf);
    loginfo[0] = LOGPOOL_EVENT_WRITE;
    loginfo[1] = size;
    loginfo += LOG_PROTOCOL_FIELDS;
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
#ifdef __cplusplus
}
#endif
