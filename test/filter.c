/*filter api test*/
#include "logpool.h"
extern logapi_t STRING_API;
DEF_LOGPOOL_PARAM_FILTER(string);
struct logpool_param_string;
static LOGPOOL_PARAM_FILTER_T(string) FILTERED_STRING_API_PARAM = {
    LOG_NOTICE,
    &STRING_API,
    {
        8,
        1024
    }
};
#define LOGAPI_PARAM cast(logpool_param_t *, &FILTERED_STRING_API_PARAM)
#define LOGAPI FILTER_API
#include "test_main.c"
