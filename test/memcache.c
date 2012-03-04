/*memcache api test*/
#include "logpool.h"
static struct logpool_param_memcache MEMCACHE_API_PARAM = {
    LOG_NOTICE,
    8,
    1024,
    "localhost",
    11211L
};
#define LOGAPI_PARAM cast(logpool_param_t *, &MEMCACHE_API_PARAM)
#define LOGAPI MEMCACHE_API
#include "test_main.c"
