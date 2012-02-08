/* syslog api test */
#include "logpool.h"
static struct logpool_param_syslog SYSLOG_API_PARAM = {8, 1024};
#define LOGAPI_PARAM cast(struct logpool_param *, &SYSLOG_API_PARAM)
#define LOGAPI SYSLOG_API
#include "test_main.c"
