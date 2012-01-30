/*memcache api test*/
static void *MEMCACHE_API_PARAM[] = {
    (void*) 1024,
    (void*) "localhost",
    (void*) 11211L
};
#define LOGAPI_PARAM MEMCACHE_API_PARAM
#define LOGAPI MEMCACHE_API
#include "test_main.c"
