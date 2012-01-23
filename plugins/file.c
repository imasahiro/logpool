#include "logpool.h"
#include "logpool_internal.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void *logpool_FILE_init(logctx ctx __UNUSED__, void **param)
{
    const char *F = cast(const char *, param[0]);
    FILE *fp = fopen(F, "w");
    return cast(void *, fp);
}

void logpool_FILE_null(logctx ctx, const char *key, uint64_t v __UNUSED__, sizeinfo_t info)
{
    FILE *fp = cast(FILE *, ctx->connection);
    fprintf(fp, "%s:null", key);
}

void logpool_FILE_bool(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    FILE *fp = cast(FILE *, ctx->connection);
    fprintf(fp, "%s:%s", key, (v != 0)?"true":"false");
}

void logpool_FILE_int(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    FILE *fp = cast(FILE *, ctx->connection);
    long i = cast(long, v);
    fprintf(fp, "%s:%ld", key, i);
}

void logpool_FILE_hex(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    FILE *fp = cast(FILE *, ctx->connection);
    long i = cast(long, v);
    fprintf(fp, "%s:0x%lx", key, i);
}

void logpool_FILE_float(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    FILE *fp = cast(FILE *, ctx->connection);
    double f = u2f(v);
    fprintf(fp, "%s:%f", key, f);
}

void logpool_FILE_char(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    FILE *fp = cast(FILE *, ctx->connection);
    long c = cast(long, v);
    fprintf(fp, "%s:%c", key, (char)c);
}

void logpool_FILE_string(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    FILE *fp = cast(FILE *, ctx->connection);
    char *s = cast(char *, v);
    fprintf(fp, "%s:'%s'", key, s);
}

void logpool_FILE_raw(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    FILE *fp = cast(FILE *, ctx->connection);
    char *s = cast(char *, v);
    fprintf(fp, "%s:%s", key, s);
}

void logpool_FILE_delim(logctx ctx)
{
    FILE *fp = cast(FILE *, ctx->connection);
    fprintf(fp, ",");
}

void logpool_FILE_flush(logctx ctx)
{
    FILE *fp;
    logctx_format_flush(ctx);
    fp = cast(FILE *, ctx->connection);
    fprintf(fp, "\n");
    fflush(fp);
}

struct logapi FILE_API = {
    logpool_FILE_null,
    logpool_FILE_bool,
    logpool_FILE_int,
    logpool_FILE_hex,
    logpool_FILE_float,
    logpool_FILE_char,
    logpool_FILE_string,
    logpool_FILE_raw,
    logpool_FILE_delim,
    logpool_FILE_flush,
    logpool_FILE_init,
};

#ifdef __cplusplus
}
#endif
