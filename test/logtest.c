#include "logpool.h"
#include <stdbool.h>

#ifndef LOGAPI_TEST
#define LOGTEST_STRING_API
#endif

#ifdef LOGTEST_STRING_API
static void *STRING_API_PARAM = (void*) 1024;
#define LOGAPI_PARAM STRING_API_PARAM
#define LOGAPI STRING_API
#endif
#ifdef LOGTEST_SYSLOG_API
static void *SYSLOG_API_PARAM = (void*) 1024;
#define LOGAPI_PARAM SYSLOG_API_PARAM
#define LOGAPI SYSLOG_API
#endif
#ifdef LOGTEST_FILE_API
static void *FILE_API_PARAM[] =  (void*) "LOG";
#define LOGAPI_PARAM FILE_API_PARAM
#define LOGAPI FILE_API
#endif

#ifdef LOGTEST_MEMCACHE_API
static void *MEMCACHE_API_PARAM_[] = {
    (void*) 1024,
    (void*) "localhost",
    (void*) 11211L
};
static void *MEMCACHE_API_PARAM = (void*) MEMCACHE_API_PARAM_;
#define LOGAPI_PARAM MEMCACHE_API_PARAM
#define LOGAPI MEMCACHE_API
#endif

extern logapi_t LOGAPI;

/* test case */
static void logpool_test_write(void)
{
#if 0
    struct logctx lctx = {};
    bool   b = true;
    long   i = 128;
    double f = 3.14;
    char  *s = "hello world";
    logctx ctx = cast(logctx, &lctx);
    logctx_init(ctx, &LOGAPI, cast(void *, "LOGDATA"));
    logctx_init_logkey(ctx, 0xcafebabe, ctx->formatter->fn_hex);

    ctx->formatter->fn_bool(ctx, "b", cast(uint64_t, b));
    ctx->formatter->fn_delim(ctx);
    ctx->formatter->fn_int(ctx, "i", cast(uint64_t ,  i));
    ctx->formatter->fn_delim(ctx);
    ctx->formatter->fn_hex(ctx, "x", cast(uint64_t ,  i));
    ctx->formatter->fn_delim(ctx);
    ctx->formatter->fn_float(ctx, "f", cast(uint64_t , f2u(f)));
    ctx->formatter->fn_delim(ctx);
    ctx->formatter->fn_string(ctx, "s", cast(uint64_t ,  s));
    ctx->formatter->fn_flush(ctx);
#endif
}

static void ltrace_test_write(ltrace_t *ltrace)
{
    double f = 3.14;
    long   i = 128;
    char  *s = "hello world";
    ltrace_record(ltrace, "event",
            LOG_f("float", f),
            LOG_i("int",   i),
            LOG_s("string", s),
            LOG_END
            );
}

static void lstate_test_write(lstate_t *state)
{
    double f = 3.14;
    long   n = 128;
    char  *s = "hello world";
    void  *p = (void*) 0xdeadbeaf;
    int i;
    for (i = 0; i < 10; ++i) {
        lstate_record(state,
                LOG_f("float", f),
                LOG_i("int",   n),
                LOG_p("ptr",   p),
                LOG_s("string", s),
                LOG_END
                );
    }
}

int main(int argc __UNUSED__, const char *argv[] __UNUSED__)
{
    ltrace_t *ltrace = ltrace_open(NULL, &LOGAPI, LOGAPI_PARAM);
    ltrace_test_write(ltrace);
    ltrace_close(ltrace);

    lstate_t *lstate = lstate_open("abcd", &LOGAPI, LOGAPI_PARAM);
    lstate_test_write(lstate);
    lstate_close(lstate);

    logpool_test_write();
    return 0;
}

