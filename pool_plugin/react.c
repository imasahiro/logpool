#include "pool_plugin.h"
#include "array.h"
#include "map.h"
#include "hash.h"

#ifdef __cplusplus
extern "C" {
#endif

union u32 {
    uint32_t v;
    uint16_t t[2];
};

static int pmap_record_val_eq(pmap_record_t *rec, char *d1, uint16_t vlen)
{
    struct LogEntry *log = (struct LogEntry *) rec->v;
    union u32 val;
    val.v = rec->v2;
    char *d0 = (char *) log + val.t[0];
    return val.t[1] != vlen || memcmp(d0, d1, vlen);
}

static int update_current_data(poolmap_t *map, struct LogEntry *e)
{
    uint16_t i, klen, vlen;
    int update = 0;
    struct Log *log = (struct Log *)&(e->data);
    char *data = log_get_data(log);
    IncRC(e, log->logsize);
    for (i = 0; i < log->logsize; ++i) {
        union u32 val;
        char *next = log_iterator(log, data, i);
        klen = log_get_length(log, i*2+0);
        vlen = log_get_length(log, i*2+1);
        val.t[0] = (data+klen/*=val*/) - ((char*)e);
        val.t[1] = vlen;

        pmap_record_t r;
        r.v  = (uintptr_t) e;
        r.v2 = (uint32_t)  val.v;
        if (poolmap_set2(map, data, klen, &r) == POOLMAP_UPDATE) {
            update |= pmap_record_val_eq(&r, data+klen, vlen);
        }
        data = next;
    }
    return update;
}

static bool react_entry_append_log(struct react *re, struct LogEntry *log)
{
    //TODO
    //re
    return update_current_data(re->map, log);
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

static void entry_log_free(pmap_record_t *r)
{
    struct LogEntry *e = (struct LogEntry *) r->v;
    DecRC(e);
    r->v = 0;
}

static void react_init(struct react *re, char *traceName, const char *key)
{
    re->map = poolmap_new(4, entry_keygen, entry_key_cmp, entry_log_free);
    re->traceID0 = djbhash(traceName, strlen(traceName));
    re->traceID1 = hash0(38873, traceName, strlen(traceName));
    re->keyID0 = djbhash(key, strlen(key));
    re->keyID1 = hash0(38873, key, strlen(key));
}

static void react_delete(struct react *re)
{
    poolmap_delete(re->map);
}

static bool react_apply(struct pool_plugin *_p, struct LogEntry *e, uint32_t state)
{
    struct pool_plugin_react *p = (struct pool_plugin_react *) _p;
    struct Log *log = (struct Log*)&e->data;
    uint16_t traceLen  = log_get_length(log, 1);
    char    *traceName = log_get_data(log) + log_get_length(log, 0);
    uint32_t traceID0 = djbhash(traceName, traceLen);
    uint32_t traceID1 = hash0(38873, traceName, traceLen);
    if (p->r.traceID0 == traceID0 && p->r.traceID1 == traceID1) {
        int update = (react_entry_append_log(&p->r, e));
        //struct pool_plugin *target = update?_p->apply:_p->failed;;
        if (update) {
            p->base.apply->Apply(_p->apply, e, update);
        } else {
            p->base.failed->Failed(_p->failed, e, update);
        }
    }
    return true;
}

static bool react_failed(struct pool_plugin *_p, struct LogEntry *e, uint32_t state)
{
    //TODO
    return true;
}

static struct pool_plugin *pool_plugin_react_create(struct pool_plugin *_p)
{
    struct pool_plugin_react *p = (struct pool_plugin_react *) _p;
    p->base.apply  = pool_plugin_init(_p->apply);
    p->base.failed = pool_plugin_init(_p->failed);
    p->base.Apply  = react_apply;
    p->base.Failed = react_failed;
    p->base.name = "react";
    react_init(&p->r, p->conf.traceName, p->conf.key);
    return _p;
}

static void pool_plugin_react_dispose(struct pool_plugin *_p)
{
    struct pool_plugin_react *p = (struct pool_plugin_react *) _p;
    CHECK_PLUGIN("react", _p);
    pool_plugin_dispose(p->base.apply);
    pool_plugin_dispose(p->base.failed);
    react_delete(&p->r);
    bzero(p, sizeof(struct pool_plugin_react));
    free(p);
}

struct pool_plugin_react pool_plugin_react_global = {
    {0, NULL, NULL, pool_plugin_react_create, pool_plugin_react_dispose, react_apply, react_failed, NULL}
};

#ifdef __cplusplus
}
#endif
