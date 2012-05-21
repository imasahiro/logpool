#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static inline char *log_iterator(struct Log *log, char *cur, uint16_t idx)
{
    uint16_t klen = log_get_length(log, idx*2+0);
    uint16_t vlen = log_get_length(log, idx*2+1);
    return cur + klen + vlen;
}

static inline void dump_log(FILE *fp, char *prefix, struct Log *log, char *suffix, int force)
{
    if (LIO_DEBUG || force) {
        int i;
        char *data = log_get_data(log);
        uint16_t klen, vlen;
        fprintf(fp, "%s", prefix);
        for (i = 0; i < log->logsize; ++i) {
            char kbuf[64] = {};
            char vbuf[64] = {};
            char *next = log_iterator(log, data, i);
            klen = log_get_length(log, i*2+0);
            vlen = log_get_length(log, i*2+1);
            memcpy(kbuf, data,klen);
            memcpy(vbuf, data+klen, vlen);
            fprintf(fp, "%d, %d, '%s': '%s' ",
                    klen, vlen, kbuf, vbuf);
            data = next;
        }
        fprintf(fp, "%s", suffix);
    }
}

#endif /* end of include guard */
