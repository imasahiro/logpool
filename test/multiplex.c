/*filter api test*/
#include "logpool.h"
extern logapi_t STRING_API;
extern logapi_t FILE2_API;
extern logapi_t MULTIPLEX_API;
static struct logpool_param_string STRING_API_PARAM = {8, 1024};
static struct logpool_param_file FILE_API_PARAM = {
    8,
    1024,
    "LOG"
};

static struct logpool_param_multiplexer MULTIPREXED_STRING_FILE_API_PARAM = {
    8,
    2,
    {
        {&STRING_API, (struct logpool_param*)&STRING_API_PARAM},
        {&FILE2_API,  (struct logpool_param*)&FILE_API_PARAM}
    }
};

#define LOGAPI_PARAM  cast(logpool_param_t *, &MULTIPREXED_STRING_FILE_API_PARAM)
#define LOGAPI_PARAM2 cast(logpool_param_t *, &MULTIPREXED_STRING_FILE_API_PARAM)
#include <stdbool.h>

static void ltrace_test_write0(ltrace_t *ltrace)
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

static void ltrace_test_write1(void)
{
    int j;
    double f = 3.14;
    long   i = 128;
    const char *s = "hello world";
    ltrace_t *ltrace = ltrace_open(NULL, &MULTIPLEX_API, LOGAPI_PARAM2);
    for (j = 0; j <= LOG_DEBUG; ++j) {
        ltrace_record(ltrace, j, "event1",
                LOG_f("float", f),
                LOG_i("int",   i),
                LOG_s("string", s),
                LOG_END
                );
    }
    ltrace_close(ltrace);
}

int main(void)
{
    logpool_init(LOGPOOL_DEFAULT);
    {
        ltrace_t *ltrace = ltrace_open(NULL, &MULTIPLEX_API, LOGAPI_PARAM);
        int i;
        for (i = 0; i < 5; ++i) {
            ltrace_test_write0(ltrace);
            ltrace_test_write1();
        }
        ltrace_close(ltrace);
    }
    return 0;
}
