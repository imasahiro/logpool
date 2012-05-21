#include "query.h"
#include "protocol.h"
#include <event2/bufferevent.h>

#ifndef LIO_UTIL_H
#define LIO_UTIL_H

static inline void do_bzero(void *ptr, size_t size)
{
    memset(ptr, 0, size);
}

static inline int emit_message(char *buf, uint16_t protocol, uint16_t logsize, ...)
{
    va_list ap;
    char *key, *val;
    uint16_t i, klen, vlen, total_size = 0;
    uint16_t *loginfo;
    struct Log *tmp = (struct Log *) buf;

    va_start(ap, logsize);
    tmp->protocol = protocol;
    tmp->logsize  = logsize;
    loginfo = ((uint16_t*)buf) + LOG_PROTOCOL_FIELDS;
    buf = (char *) (loginfo + logsize * 2);
    for (i = 0; i < logsize; ++i) {
        klen = (uint16_t) va_arg(ap, unsigned long);
        vlen = (uint16_t) va_arg(ap, unsigned long);
        key  = va_arg(ap, char *);
        val  = va_arg(ap, char *);
        loginfo[i*2+0] = klen;
        loginfo[i*2+1] = vlen;
        if (klen) {
            memcpy(buf, key, klen);
            buf = buf + klen;
        }
        if (vlen) {
            memcpy(buf, val, vlen);
            buf = buf + vlen;
        }
        total_size += klen + vlen;
    }
    return LOG_PROTOCOL_SIZE + sizeof(uint16_t) * logsize * 2 + total_size;
}

static inline int util_send_quit_msg(struct bufferevent *bev)
{
    char buf[16];
    int size = emit_message(buf, LOGPOOL_EVENT_QUIT, 1, 0, 0, NULL, NULL);
    if (bufferevent_write(bev, buf, size) != 0) {
        fprintf(stderr, "[util:quit] write error\n");
        return LIO_FAILED;
    }
    return LIO_OK;
}

#endif /* end of include guard */
