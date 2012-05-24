#define MEMORY_PREFIX react
#include "memory.h"
#undef MEMORY_PREFIX
#include "map.h"
#include "hash.h"
#include "reactive.h"
#include "lio/lio.h"
#include "lio/protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RENGINE_ENTRY_INITSIZE 16

#ifdef __cplusplus
extern "C" {
#endif

struct LogEntry {
    struct LogEntry *next;
    struct Message data;
};

DEF_ARRAY_OP(react_watcher_t);
DEF_ARRAY_OP(reaction_entry_t);

static void react_entry_append_log(react_engine_t *re, reaction_entry_t *e, struct Log *logbuf, uint32_t logsize)
{
    react_watcher_t *w, *we;
    FOR_EACH_ARRAY(e->watcher, w, we) {
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
    react_watcher_t *w, *we;
    FOR_EACH_ARRAY(entry->watcher, w, we) {
        w->remove(w->data);
    }
    ARRAY_dispose(react_watcher_t, &entry->watcher);
}

void react_engine_append(react_engine_t *re, char *key, uint32_t klen, reaction_entry_t *entry)
{
    reaction_entry_t *e = cast(reaction_entry_t *, do_malloc(sizeof(*e)));
    memcpy(e, entry, sizeof(*e));
    poolmap_set(re->react_entries, key, klen, e);
    ARRAY_init(react_watcher_t, &e->watcher, 2);
}

static int entry_key_cmp(uintptr_t k0, uintptr_t k1)
{
    return k0 == k1;
}

static uintptr_t entry_keygen(char *key, uint32_t klen)
{
#if 0
    uint32_t ukey = *(uint32_t *) key;
    uint32_t mask = ~(((uint32_t)-1) << (klen * 8));
    uint32_t hash = ukey & mask;
#else
    uint32_t hash = 38873;
#endif
    return hash0(hash, key, klen);
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
    re->react_entries = poolmap_new(entry_size, entry_keygen, entry_key_cmp, entry_free);
    return re;
}

void react_engine_delete(react_engine_t *re)
{
    poolmap_delete(re->react_entries);
    do_free(re, sizeof(react_engine_t));
    CHECK_MALLOCED_SIZE(react);
}

#ifdef __cplusplus
}
#endif
