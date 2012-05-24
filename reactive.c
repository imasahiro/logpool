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
#define TEST_ENTRY   1000000
#define TEST_WATCHER 100000
#define TEST_LOG     100000
#define TEST_WATCHER_PER_ENTRY 100

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
    poolmap_t *react_entries;
    //ARRAY(reaction_entry_t) entries;
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
    uint16_t traceLen  = log_get_length(logbuf, 1);
    char    *traceName = log_get_data(logbuf) + log_get_length(logbuf, 0);

    pmap_record_t *rec;
    if ((rec = poolmap_get(re->react_entries, traceName, traceLen))) {
        struct reaction_entry *e = (struct reaction_entry *) rec->v;
        react_entry_append_log(re, e, logbuf, logsize);
    }
}

void react_engine_append_watcher(react_engine_t *re, char *key, uint32_t klen, react_watcher_t *watcher)
{
    pmap_record_t *rec;
    if ((rec = poolmap_get(re->react_entries, key, klen))) {
        struct reaction_entry *e = (struct reaction_entry *) rec->v;
        ARRAY_add(react_watcher_t, &e->watcher, watcher);
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

void react_engine_append(react_engine_t *re, char *key, uint32_t klen, reaction_entry_t *entry)
{
    reaction_entry_t *e = cast(reaction_entry_t *, do_malloc(sizeof(*e)));
    memcpy(e, entry, sizeof(*e));
    poolmap_set(re->react_entries, key, klen, e);
    //ARRAY_add(reaction_entry_t, &re->entries, entry);
    ARRAY_init(react_watcher_t, &e->watcher, 1);
}

static int entry_key_cmp(uintptr_t k0, uintptr_t k1)
{
    char *s0 = (char *) k0;
    char *s1 = (char *) k1;
    return strcmp(s0, s1) == 0;
}

static void entry_free(pmap_record_t *r)
{
    reaction_entry_t *e = cast(reaction_entry_t *, r->v);
    reaction_entry_reset(e);
    do_free(e, sizeof(reaction_entry_t));
}

react_engine_t *react_engine_new(unsigned int entry_size)
{
    react_engine_t *re = cast(react_engine_t *, do_malloc(sizeof(*re)));
    if (entry_size < RENGINE_ENTRY_INITSIZE)
        entry_size = RENGINE_ENTRY_INITSIZE;
    re->react_entries = poolmap_new(entry_size, entry_key_cmp, entry_free);
    return re;
}

void react_engine_delete(react_engine_t *re)
{
    uint32_t i;
    struct reaction_entry *e;
    poolmap_delete(re->react_entries);
    do_free(re, sizeof(react_engine_t));
}

static void watcher_watch(uintptr_t data)
{
    intptr_t *sum = (intptr_t *) data;
    *sum += 1;
}

static void watcher_remove(uintptr_t data)
{
    intptr_t *sum = (intptr_t *) data;
    *sum -= 1;
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

int main(int argc, char const* argv[])
{
    react_engine_t *re = react_engine_new(4);
    reaction_entry_t e = {};
    intptr_t sum = 0;
    react_watcher_t  w = {(intptr_t)&sum, 10, watcher_watch, watcher_remove};
    int i;
    for (i = 0; i < TEST_ENTRY; ++i) {
        char data[128] = {};
        snprintf(data, 128, "%d", i % TEST_WATCHER_PER_ENTRY);
        react_engine_append(re, data, strlen(data), &e);
    }
    fprintf(stderr, "mapsize=%d\n", poolmap_size(re->react_entries));
    for (i = 0; i < TEST_WATCHER; ++i) {
        char data[128] = {};
        snprintf(data, 128, "%d", i % TEST_WATCHER_PER_ENTRY);
        react_engine_append_watcher(re, data, strlen(data), &w);
    }
    for (i = 0; i < TEST_LOG; ++i) {
        emit_log(re, i % TEST_WATCHER_PER_ENTRY);
    }

    react_engine_delete(re);
    fprintf(stderr, "sum=%ld\n", sum);
    //assert(sum == 0);
    fprintf(stderr, "%ld\n", sizeof(struct reaction_entry));
    CHECK_MALLOCED_SIZE(react);
    return 0;
}
