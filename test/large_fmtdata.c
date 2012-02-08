/* large size format test */
#include "logpool.h"
static struct logpool_param_string STRING_API_PARAM = {
#define LOGFMT_MAX_SIZE 32
    LOGFMT_MAX_SIZE,
    1024
};
#define LOGAPI_PARAM cast(struct logpool_param *, &STRING_API_PARAM)
#define LOGAPI STRING_API

#include "logpool.h"
#include <stdbool.h>
extern logapi_t LOGAPI;

static void ltrace_test_write(ltrace_t *ltrace)
{
    double f = 3.14;
    long   i = 128;
    const char *s = "hello world";
    ltrace_record(ltrace, "event",
            LOG_f("0:float", f),
            LOG_i("0:int",   i),
            LOG_s("0:string", s),
            LOG_f("1:float", f),
            LOG_i("1:int",   i),
            LOG_s("1:string", s),
            LOG_f("2:float", f),
            LOG_i("2:int",   i),
            LOG_s("2:string", s),
            LOG_f("3:float", f),
            LOG_i("3:int",   i),
            LOG_s("3:string", s),
            LOG_f("4:float", f),
            LOG_i("4:int",   i),
            LOG_s("4:string", s),
            LOG_f("5:float", f),
            LOG_i("5:int",   i),
            LOG_s("5:string", s),
            LOG_f("6:float", f),
            LOG_i("6:int",   i),
            LOG_s("6:string", s),
            LOG_f("7:float", f),
            LOG_i("7:int",   i),
            LOG_s("7:string", s),
            LOG_f("8:float", f),
            LOG_i("8:int",   i),
            LOG_s("8:string", s),
            LOG_f("9:float", f),
            LOG_i("9:int",   i),
            LOG_s("9:string", s),
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
                LOG_f("0:float", f),
                LOG_i("0:int",   n),
                LOG_p("0:ptr",   p),
                LOG_s("0:string", s),
                LOG_f("1:float", f),
                LOG_i("1:int",   n),
                LOG_p("1:ptr",   p),
                LOG_s("1:string", s),
                LOG_f("2:float", f),
                LOG_i("2:int",   n),
                LOG_p("2:ptr",   p),
                LOG_s("2:string", s),
                LOG_f("3:float", f),
                LOG_i("3:int",   n),
                LOG_p("3:ptr",   p),
                LOG_s("3:string", s),
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
