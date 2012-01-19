#include "logpool.h"
#include <stdbool.h>

#define LOGTEST_LLVM_STRING_API
#ifdef LOGTEST_LLVM_STRING_API
static void *LLVM_STRING_API_PARAM[] = {(void*) 1024};
#define LOGAPI_PARAM LLVM_STRING_API_PARAM
#define LOGAPI LLVM_STRING_API
#endif

extern logapi_t LOGAPI;

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
    logpool_init(LOGPOOL_JIT);
    lstate_t *lstate = lstate_open("abcd", &LOGAPI, LOGAPI_PARAM);
    lstate_test_write(lstate);
    lstate_close(lstate);
    return 0;
}

