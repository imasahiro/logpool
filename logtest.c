#include "logpool.h"
#include <stdbool.h>

#define LOGAPI SYSLOG_API
extern logapi_t LOGAPI;

void logpool_test_write(void)
{
    struct logctx lctx = {};
    bool   b = true;
    long   i = 128;
    double f = 3.14;
    char  *s = "hello world";
    logctx ctx = cast(logctx, &lctx);
    logctx_init(ctx, &LOGAPI, cast(void *, "LOGDATA"));
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
}

void ltrace_test_write(ltrace_t *ltrace)
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

void lstate_test_write(lstate_t *state)
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

int main(int argc, const char *argv[])
{
    ltrace_t *ltrace = ltrace_open(NULL, &LOGAPI);
    ltrace_test_write(ltrace);
    ltrace_close(ltrace);

    lstate_t *lstate = lstate_open("abcd", &LOGAPI);
    lstate_test_write(lstate);
    lstate_close(lstate);

    logpool_test_write();
    return 0;
}

