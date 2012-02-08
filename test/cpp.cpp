/* string api test */
#include "logpool.h"
static struct logpool_param_string STRING_API_PARAM = {8, 1024};
#define LOGAPI_PARAM cast(struct logpool_param *, &STRING_API_PARAM)
#define LOGAPI STRING_API
#include "test_main.c"
