/*filter api test*/
#include "logpool.h"
extern logapi_t STRING_API;
static struct logpool_param_string STRING_API_PARAM = {8, 1024};
static struct logpool_param_filter FILTERED_STRING_API_PARAM = {
    8, LOG_NOTICE, &STRING_API, (struct logpool_param *) &STRING_API_PARAM
};
#define LOGAPI_PARAM cast(logpool_param_t *, &FILTERED_STRING_API_PARAM)
#define LOGAPI FILTER_API

#include <stdbool.h>

#ifndef LOGAPI
#error define LOGAPI && LOGAPI_PARAM
#endif

extern logapi_t LOGAPI;

static void ltrace_test_write0(ltrace_t *ltrace)
{
    double f = 3.14;
    long   i = 128;
    const char *s = "hello world";
    ltrace_record(ltrace, LOG_INFO, "event",
            LOG_f("float", f),
            LOG_i("int",   i),
            LOG_s("string", s),
            LOG_END
            );
}

static void ltrace_test_write1(ltrace_t *ltrace)
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
        int i;
        for (i = 0; i < 5; ++i) {
            ltrace_test_write0(ltrace);
            ltrace_test_write1(ltrace);
        }
        ltrace_close(ltrace);
    }
    return 0;
}
