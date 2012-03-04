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
    ltrace_record(ltrace, LOG_NOTICE, "event",
            LOG_f("float", f),
            LOG_i("int",   i),
            LOG_s("string", s),
            LOG_END
            );
}

int main(void)
{
    logpool_init(LOGPOOL_DEFAULT);
    {
        ltrace_t *ltrace = ltrace_open(NULL, &LOGAPI, LOGAPI_PARAM);
        ltrace_test_write(ltrace);
        ltrace_close(ltrace);
    }
    return 0;
}
