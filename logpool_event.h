#ifndef LOGPOOL_EVENT_H_
#define LOGPOOL_EVENT_H_

enum LOGPOOL_EVENT_PROTOCOL {
    LOGPOOL_EVENT_NULL,
    LOGPOOL_EVENT_WRITE,
    LOGPOOL_EVENT_READ,
    LOGPOOL_EVENT_QUIT
};

struct logpool_protocol {
    uint16_t protocol, klen;
    uint16_t vlen;
};

#endif /* end of include guard */
