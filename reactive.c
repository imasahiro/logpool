#define MEMORY_PREFIX react
#include "memory.h"
#undef MEMORY_PREFIX
#include "map.h"
#include "hash.h"
#include "array.h"
#include "lio/lio.h"
#include "lio/protocol.h"
#include "lio/util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RENGINE_ENTRY_INITSIZE 16
#define cast(T, V) ((T)(V))

struct filter;
struct LogEntry {
    struct LogEntry *next;
    struct Message data;
};

typedef struct react_watcher {
    uintptr_t data;
    uintptr_t expire_time;
    void (*watch) (uintptr_t);
    void (*remove)(uintptr_t);
} react_watcher_t;

DEF_ARRAY_STRUCT0(react_watcher_t, uint32_t);
DEF_ARRAY_T_OP(react_watcher_t);

typedef struct reaction_entry {
    uint32_t traceID;
    uint16_t logsize;
    ARRAY(react_watcher_t) watcher;
    struct LogEntry *logHead;
    struct LogEntry *logTail;
} reaction_entry_t;

DEF_ARRAY_STRUCT0(reaction_entry_t, uint32_t);
DEF_ARRAY_T_OP(reaction_entry_t);

typedef struct react_engine {
    poolmap_t *pmap;
    ARRAY(reaction_entry_t) entries;
    void *memory;
} react_engine_t;

static void react_entry_append_log(react_engine_t *re, reaction_entry_t *e, struct Log *logbuf, uint32_t logsize)
{
    uint32_t i;
    react_watcher_t *w;
    FOR_EACH_ARRAY(e->watcher, w, i) {
        w->watch(w->data);
    }
}

void react_engine_append_log(react_engine_t *re, struct Log *logbuf, uint32_t logsize)
{
    int i;
    struct reaction_entry *e;
    uint16_t traceLen  = log_get_length(logbuf, 1);
    char    *traceName = log_get_data(logbuf) + log_get_length(logbuf, 0);
    uint32_t traceID   = djbhash(traceName, traceLen) % 10;

    //fprintf(stderr, "%d %s, %x\n", traceLen, traceName, traceID);
    FOR_EACH_ARRAY(re->entries, e, i) {
        if (e->traceID == traceID) {
            react_entry_append_log(re, e, logbuf, logsize);
            return;
        }
    }
}

void react_engine_append_watcher(react_engine_t *re, uint32_t traceID, react_watcher_t *watcher)
{
    int i;
    struct reaction_entry *e;
    FOR_EACH_ARRAY(re->entries, e, i) {
        if (e->traceID == traceID) {
            ARRAY_add(react_watcher_t, &e->watcher, watcher);
            break;
        }
    }
}

static void reaction_entry_reset(struct reaction_entry *entry)
{
    uint32_t i;
    react_watcher_t *w;
    FOR_EACH_ARRAY(entry->watcher, w, i) {
        w->remove(w->data);
    }
    ARRAY_dispose(react_watcher_t, &entry->watcher);
}

void react_engine_append(react_engine_t *re, reaction_entry_t *entry)
{
    ARRAY_add(reaction_entry_t, &re->entries, entry);
    reaction_entry_t *e = ARRAY_last(re->entries);
    ARRAY_init(react_watcher_t, &e->watcher, 4);
}

react_engine_t *react_engine_new(unsigned int entry_size)
{
    react_engine_t *re = cast(react_engine_t *, do_malloc(sizeof(*re)));
    if (entry_size < RENGINE_ENTRY_INITSIZE)
        entry_size = RENGINE_ENTRY_INITSIZE;
    ARRAY_init(reaction_entry_t, &re->entries, entry_size);
    return re;
}

void react_engine_delete(react_engine_t *re)
{
    uint32_t i;
    struct reaction_entry *e;
    FOR_EACH_ARRAY(re->entries, e, i) {
        reaction_entry_reset(e);
    }
    ARRAY_dispose(reaction_entry_t, &re->entries);
    do_free(re, sizeof(react_engine_t));
}

intptr_t sum = 0;
static void watcher_watch(uintptr_t data)
{
    sum += data;
}

static void watcher_remove(uintptr_t data)
{
    sum -= data;
}

static void emit_log(react_engine_t *re, int i)
{
    char buf[1024];
    char data[128];
    snprintf(data, 128, "%d", i);
    int logSize = emit_message(buf, LOGPOOL_EVENT_WRITE, 3,
            strlen("TraceID"), strlen(data), "TraceID", data,
            strlen("key0"), strlen("val0"), "key0", "val0",
            strlen("key1"), strlen("val1"), "key1", "val1",
            strlen("key2"), strlen("val2"), "key2", "val2");
    react_engine_append_log(re, (struct Log *) buf, logSize);
}
#define TEST_ENTRY   100
#define TEST_WATCHER 100000
#define TEST_LOG     100000
int main(int argc, char const* argv[])
{
    react_engine_t *re = react_engine_new(4);
    reaction_entry_t e = {};
    react_watcher_t  w = {0, 10, watcher_watch, watcher_remove};
    int i;
    for (i = 0; i < TEST_ENTRY; ++i) {
        e.traceID = i % 10;
        react_engine_append(re, &e);
    }
    uint32_t traceID = 0;
    for (i = 0; i < TEST_WATCHER; ++i) {
        traceID = i % 10;
        w.data  = traceID;
        react_engine_append_watcher(re, traceID, &w);
    }
    for (i = 0; i < TEST_LOG; ++i) {
        emit_log(re, i % 10);
    }

    react_engine_delete(re);
    fprintf(stderr, "%lld\n", sum);
    //assert(sum == 0);
    CHECK_MALLOCED_SIZE(react);
    fprintf(stderr, "%ld\n", sizeof(struct reaction_entry));
    return 0;
}
