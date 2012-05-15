#include <stdbool.h>

#ifndef LOGAPI
#error define LOGAPI && LOGAPI_PARAM
#endif
#ifndef LOGPOOL_TEST_COUNT
#define LOGPOOL_TEST_COUNT(argc, argv) 5
#endif
extern logapi_t LOGAPI;
#define LOG_END 0
#define LOG_s   1
#define LOG_u   2

#define KEYVALUE_u(K,V)    LOG_u, (K), strlen(K), ((uintptr_t)V), 0
#define KEYVALUE_s(K,V)    LOG_s, (K), strlen(K), (V), strlen(V)


int n = 0;
static void logpool_test_write(logpool_t *logpool)
{
    //double f = n + 0.14;
    long   i = n;
    const char *s = "hello world";
    void *args;
    logpool_record(logpool, &args, LOG_NOTICE, "event",
            KEYVALUE_u("uint",   i),
            KEYVALUE_u("uint",   i+1),
            KEYVALUE_u("uint",   i*10),
            KEYVALUE_s("string", s),
            LOG_END
            );
    n++;

}

int main(int argc, char const* argv[])
{
    logpool_init(LOGAPI_INIT_FLAG);
    logpool_t *logpool = logpool_open(NULL, &LOGAPI, LOGAPI_PARAM);
    int i, size = LOGPOOL_TEST_COUNT(argc, argv);
    for (i = 0; i < size; ++i) {
        logpool_test_write(logpool);
    }
    logpool_close(logpool);
    logpool_exit();
    return 0;
}
