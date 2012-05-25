#include "array.h"
#include "map.h"
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include "lio/lio.h"
#include "lio/protocol.h"

#ifndef LOGPOOL_REACTIVE_H
#define LOGPOOL_REACTIVE_H

#ifdef __cplusplus
extern "C" {
#endif

struct Log;
struct LogEntry {
    struct LogHead {
        struct LogEntry *next;
        uint64_t time;
        uint16_t size;
        int16_t  refc;
    } h;
    struct Message data;
};

typedef struct react_watcher {
    uintptr_t data;
    uintptr_t expire_time;
    void (*watch) (uintptr_t, struct LogEntry *);
    void (*remove)(uintptr_t);
} react_watcher_t;

DEF_ARRAY_STRUCT0(react_watcher_t, uint32_t);
DEF_ARRAY_T(react_watcher_t);

struct LogList {
    struct LogEntry *head;
    struct LogEntry *tail;
};

typedef struct reaction_entry {
    uint32_t traceID;
    uint32_t logsize;
    uint64_t expire_time;
    ARRAY(react_watcher_t) watcher;
    struct LogList logs;
    poolmap_t *map;
} reaction_entry_t;

DEF_ARRAY_STRUCT0(reaction_entry_t, uint32_t);
DEF_ARRAY_T(reaction_entry_t);

typedef struct react_engine {
    poolmap_t *react_entries;
    poolmap_t *pmap;
} react_engine_t;

react_engine_t *react_engine_new(unsigned int entry_size);
void react_engine_delete(react_engine_t *re);

void react_engine_append_log(react_engine_t *re, struct Log *logbuf, uint32_t logsize);
void react_engine_append_watcher(react_engine_t *re, char *key, uint32_t klen, react_watcher_t *watcher);
void react_engine_append(react_engine_t *re, char *key, uint32_t klen, reaction_entry_t *entry);

static inline uint64_t TimeMilliSecond(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec;
}

#ifdef __cplusplus
}
#endif

#endif /* end of include guard */
