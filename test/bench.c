#define N 10000000
#include "logpool.h"

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

static inline uint64_t getTimeMilliSecond(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

uint64_t logpool_ntrace1(ltrace_t *ltrace)
{
    int pid  = 0x10;
    int pgid = 0x20;
    void *p  = (void*) 0xdeadbeaf;
    int i;
    uint64_t s = getTimeMilliSecond();
    for (i = 0; i < N; ++i) {
        ltrace_record(ltrace, "setpgid", LOG_i("pid", pid), LOG_i("pgid", pgid), LOG_p("ptr", p), LOG_END);
    }
    uint64_t e = getTimeMilliSecond();
    return e - s;
}
uint64_t logpool_ntrace0(ltrace_t *ltrace)
{
    int i;
    uint64_t s = getTimeMilliSecond();
    for (i = 0; i < N; ++i) {
        ltrace_record(ltrace, "setpgid", LOG_END);
    }
    uint64_t e = getTimeMilliSecond();
    return e - s;
}

extern logapi_t FILE2_API;
static logapi_t *API = &FILE2_API;
int main(int argc, const char *argv[])
{
    static void *ARGS[] = {
        (void *) 1024,
        (void *) "/dev/null"
    };
    ltrace_t *ltrace = ltrace_open(NULL, API, ARGS);
    uint64_t t1 = logpool_ntrace0(ltrace);
    uint64_t t2 = logpool_ntrace1(ltrace);
    fprintf(stderr, "logpool0:%lld\n", t1);
    fprintf(stderr, "logpool1:%lld\n", t2);
    ltrace_close(ltrace);
    return 0;
}

