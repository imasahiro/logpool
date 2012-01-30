#include "logpool.h"
#include <stdbool.h>

#ifndef LOGAPI
#error define LOGAPI && LOGAPI_PARAM
#endif

extern logapi_t LOGAPI;

static void ltrace_test_write(ltrace_t *ltrace)
{
    double f = 3.14;
    long   i = 128;
    const char *s = "hello world";
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
    const char *s = "hello world";
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
    logpool_init(LOGPOOL_DEFAULT);
    {
        ltrace_t *ltrace = ltrace_open(NULL, &LOGAPI, LOGAPI_PARAM);
        ltrace_test_write(ltrace);
        ltrace_close(ltrace);
    }

    {
        lstate_t *lstate = lstate_open("abcd", &LOGAPI, LOGAPI_PARAM);
        lstate_test_write(lstate);
        lstate_close(lstate);
    }
    return 0;
}
