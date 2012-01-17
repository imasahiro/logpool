#include "logpool.h"
#include "logpool_string.h"
#include <syslog.h>

static void logpool_syslog_flush(logctx ctx)
{
    logpool_string_flush(ctx);
    //syslog(LOG_NOTICE, buf->base);
    syslog(LOG_NOTICE, "%s", cast(buffer_t *, ctx->connection)->base);
}

struct logapi SYSLOG_API = {
    logpool_string_null,
    logpool_string_bool,
    logpool_string_int,
    logpool_string_hex,
    logpool_string_float,
    logpool_string_char,
    logpool_string_string,
    logpool_string_raw,
    logpool_string_delim,
    logpool_syslog_flush,
    logpool_string_init,
};

