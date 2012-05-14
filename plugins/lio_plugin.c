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
#define DEFAULT_PORT   10000

#ifdef __cplusplus
extern "C" {
#endif

static struct lio *g_lio = NULL;
struct lio_plugin {
    char *buf;
    struct lio *lio;
    char base[1];
};

#if 0
typedef struct logpool logpool_t;
logpool_t *logpool_open(logpool_t *parent, struct logapi *api, logpool_param_t *);
intptr_t logpool_record(logpool_t *, void *data, int PRIORITY, char *key, ...);
int logpool_check_priority(logpool_t *, int PRIORITY);
#endif
static void *logpool_LogPool_init(logpool_t *logpool, logpool_param_t *p)
{
    struct lio_plugin *lp = cast(struct lio_plugin *, logpool_string_init(logpool, p));
    lp->lio = g_lio;
    return cast(void *, lp);
}

static void logpool_LogPool_flush(logpool_t *logpool, void **args __UNUSED__)
{
    struct lio_plugin *lp = cast(struct lio_plugin *, logpool->connection);
    char *buf_orig = lp->buf, *p;
    size_t klen, vlen, size;
    struct logfmt *fmt, *fmte;
    int ret;

    size = logpool->logfmt_size;
    fmt = cast(struct logpool *, logpool)->fmt;
    fmte = fmt + size;
    lp->buf = lp->buf + LOG_PROTOCOL_SIZE;
    p = logpool->fn_key(logpool, logpool->logkey.v.u, logpool->logkey.k.seq, logpool->logkey.klen);
    klen = p - (char*) buf_orig - LOG_PROTOCOL_SIZE;
    if (size) {
        void (*fn_delim)(logpool_t *) = logpool->formatter->fn_delim;
        fmt->fn(logpool, fmt->k.key, fmt->v.u, fmt->klen, fmt->vlen);
        ++fmt;
        while (fmt < fmte) {
            fn_delim(logpool);
            fmt->fn(logpool, fmt->k.key, fmt->v.u, fmt->klen, fmt->vlen);
            ++fmt;
        }
        cast(struct logpool *, logpool)->logfmt_size = 0;
    }
    vlen = (char*) lp->buf - p;
    /*TODO*/
    ret = lio_write(lp->lio, buf_orig, klen);
    if (ret != LOGPOOL_SUCCESS) {
        /* TODO Error */
        fprintf(stderr, "Error!!\n");
        abort();
    }
    logpool_string_reset(logpool);
    ++(cast(struct logpool *, logpool)->logkey.k.seq);
}

struct logapi EVENT_API = {
    logpool_string_null,
    logpool_string_bool,
    logpool_string_int,
    logpool_string_hex,
    logpool_string_float,
    logpool_string_char,
    logpool_string_string,
    logpool_string_raw,
    logpool_string_delim,
    logpool_LogPool_flush,
    logpool_LogPool_init,
    logpool_string_close,
    logpool_default_priority
};

extern struct keyapi *logpool_string_api_init(void);
struct keyapi *logpool_event_api_init(void)
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
    }
    fprintf(stderr,"connect to [%s:%u]\n", host, port);
    g_lio = lio_open_trace(host, port);
    return logpool_string_api_init();
}

void logpool_event_api_deinit(void)
{
    lio_close(g_lio);
}

#ifdef __cplusplus
}
#endif
