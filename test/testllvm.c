#include "logpool.h"
#include <stdbool.h>

static void *LLVM_STRING_API_PARAM[] = {(void*) 1024};
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

int main(int argc __UNUSED__, const char *argv[] __UNUSED__)
{
    lstate_t *lstate;
    logpool_init(LOGPOOL_JIT);
    lstate = lstate_open("abcd", &LLVM_STRING_API, LLVM_STRING_API_PARAM);
    lstate_test_write(lstate);
    lstate_close(lstate);
    return 0;
}

