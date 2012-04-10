#include <stdbool.h>

#ifndef LOGAPI
#error define LOGAPI && LOGAPI_PARAM
#endif
#ifndef LOGPOOL_TEST_COUNT
#define LOGPOOL_TEST_COUNT 5
#endif
extern logapi_t LOGAPI;

int n = 0;
static void ltrace_test_write(ltrace_t *ltrace)
{
    double f = n + 0.14;
    long   i = n;
    const char *s = "hello world";
    ltrace_record(ltrace, LOG_NOTICE, "event",
            LOG_f("float", f),
            LOG_i("int",   i),
            LOG_i("int",   i+1),
            LOG_i("int",   i*10),
            //LOG_s("string", s),
            LOG_END
            );
    n++;

}

int main(void)
{
    logpool_init(LOGAPI_INIT_FLAG);
    {
        ltrace_t *ltrace = ltrace_open(NULL, &LOGAPI, LOGAPI_PARAM);
        int i;
        for (i = 0; i < LOGPOOL_TEST_COUNT; ++i) {
            ltrace_test_write(ltrace);
        }
        ltrace_close(ltrace);
    }
    logpool_exit();
    return 0;
}
