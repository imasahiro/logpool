/*event api test*/
#include "logpool.h"
#include <stdlib.h>
static struct logpool_param_memcache EVENT_API_PARAM = {
    8,
    1024,
    "localhost",
    10000L
};
#define LOGAPI_PARAM cast(logpool_param_t *, &EVENT_API_PARAM)
#define LOGAPI_INIT_FLAG (LOGPOOL_EVENT)
#define LOGPOOL_TEST_COUNT get_count()
#define LOGAPI EVENT_API
static int get_count(void)
{
    char *env = getenv("LOGPOOL_TESTCASE_SIZE");
    if (env) {
        return strtol(env, NULL, 10);
    }
    return 100;
}
#include "test_main.c"

