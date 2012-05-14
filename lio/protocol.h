#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "message.idl.data.h"
#ifndef PROTOCOL_H
#define PROTOCOL_H

enum LOGPOOL_EVENT_PROTOCOL {
    LOGPOOL_EVENT_NULL,
    LOGPOOL_EVENT_WRITE,
    LOGPOOL_EVENT_READ,
    LOGPOOL_EVENT_QUIT
};

struct log_data {
    uint16_t protocol, logsize, klen, vlen;
    char data[128];
};
#define LOG_PROTOCOL_FIELDS 2
#define LOG_PROTOCOL_SIZE (sizeof(uint16_t)*LOG_PROTOCOL_FIELDS)

static inline enum LOGPOOL_EVENT_PROTOCOL log_data_protocol(struct log_data *data)
{
    return (enum LOGPOOL_EVENT_PROTOCOL) data->protocol;
}

static inline uint16_t log_data_process(struct log_data *data)
{
    uint16_t protocol = log_data_protocol(data);
    switch (protocol) {
        case LOGPOOL_EVENT_NULL:
        case LOGPOOL_EVENT_READ:
            break;
        case LOGPOOL_EVENT_WRITE:
            debug_print(0, "%d, %d, '%s'\n", data->klen, data->vlen, data->data);
            break;
        case LOGPOOL_EVENT_QUIT:
            debug_print(0, "quit, %d, %d\n", data->klen, data->vlen);
            break;
        default:
            /*TODO*/abort();
            break;
    }
    return protocol;
}

static inline char *log_get_data(struct Log *log)
{
    int offset = LOG_PROTOCOL_FIELDS+(log->logsize)*2;
    uint16_t *p = (uint16_t*)log;
    return (char *)(p + offset);
}

static inline uint16_t log_get_length(struct Log *log, uint16_t idx)
{
    int offset = LOG_PROTOCOL_FIELDS+idx;
    return ((uint16_t*)log)[offset];
}

#endif /* end of include guard */
