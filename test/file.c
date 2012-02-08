/* file api test */
#include "logpool.h"
static struct logpool_param_file FILE_API_PARAM = {
    8,
    1024,
    "LOG"
};
#define LOGAPI_PARAM cast(struct logpool_param *, &FILE_API_PARAM)
#define LOGAPI FILE2_API
#include "test_main.c"
