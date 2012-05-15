#include "logpool.h"
#include <stdio.h>
static struct logpool_param_trace TRACE_API_PARAM = {
    8,
    1024,
    "127.0.0.1", 14801
};
#define LOGAPI_PARAM cast(logpool_param_t *, &TRACE_API_PARAM)
#define LOGAPI TRACE_API
#define LOG_END 0
#define LOG_s   1
#define LOG_u   2

#define KEYVALUE_u(K,V)    LOG_u, (K), strlen(K), ((uintptr_t)V), 0
#define KEYVALUE_s(K,V)    LOG_s, (K), strlen(K), (V), strlen(V)

extern logapi_t LOGAPI;

int main(int argc, char **argv)
{
    logpool_init(LOGPOOL_TRACE);
    logpool_t *logpool = logpool_open(NULL, &LOGAPI, LOGAPI_PARAM);
    void *logpool_args;
    if (argc < 2) {
        fprintf(stderr, "usage: %s key value key value ...\n", argv[0]);
        goto L_error;
    }
    switch (argc) {
        case 3:
            logpool_record(logpool, &logpool_args, LOG_NOTICE, "logput",
                    KEYVALUE_s(argv[1], argv[2]),
                    LOG_END);
            break;
        case 5:
            logpool_record(logpool, &logpool_args, LOG_NOTICE, "logput",
                    KEYVALUE_s(argv[1], argv[2]),
                    KEYVALUE_s(argv[3], argv[4]),
                    LOG_END);
            break;
        case 7:
            logpool_record(logpool, &logpool_args, LOG_NOTICE, "logput",
                    KEYVALUE_s(argv[1], argv[2]),
                    KEYVALUE_s(argv[3], argv[4]),
                    LOG_END);
            break;
    }
    logpool_close(logpool);
    logpool_exit();
    return 0;
L_error:;
    logpool_close(logpool);
    logpool_exit();
    return 1;
}
