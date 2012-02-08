#include "logpool.h"
#include <stdbool.h>

static struct logpool_param_string LLVM_STRING_API_PARAM = {8, 1024};
extern logapi_t LLVM_STRING_API;

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

void lstate_test(void)
{
    lstate_t *lstate;
    logpool_init(LOGPOOL_JIT);
    lstate = lstate_open("abcd", &LLVM_STRING_API, (logpool_param_t *) &LLVM_STRING_API_PARAM);
    lstate_test_write(lstate);
    lstate_close(lstate);
}

void ltrace_test(void) {
    ltrace_t *ltrace;
    const char *s = "hello world";
    logpool_init(LOGPOOL_JIT);
    ltrace = ltrace_open(NULL, &LLVM_STRING_API,
            (logpool_param_t *) &LLVM_STRING_API_PARAM);
    ltrace_record(ltrace, "test",
            LOG_s("string", s),
            LOG_END
            );
    ltrace_close(ltrace);
}

int main(void)
{
    ltrace_test();
    lstate_test();
    return 0;
}
