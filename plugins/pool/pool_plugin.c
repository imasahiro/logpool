#include "pool_plugin.h"
#include "array.h"
#include "protocol.h"
#include "konoha2/konoha2.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

static void pool_plugin_nop_dispose(struct pool_plugin *p);
static struct pool_plugin *pool_plugin_nop_create(struct pool_plugin *_p);
static bool nop_apply(struct pool_plugin *p, struct LogEntry *e, uint32_t state);
static bool nop_failed(struct pool_plugin *p, struct LogEntry *e, uint32_t state);

struct pool_plugin_nop {
    struct pool_plugin base;
};

struct pool_plugin_nop pool_nop_plugin = {
    {0, NULL, NULL, pool_plugin_nop_create, pool_plugin_nop_dispose, nop_apply, nop_failed, "nop"}
};

static bool nop_apply(struct pool_plugin *p, struct LogEntry *e, uint32_t state)
{
    return true;
}

static bool nop_failed(struct pool_plugin *p, struct LogEntry *e, uint32_t state)
{
    return true;
}

static struct pool_plugin *pool_plugin_nop_create(struct pool_plugin *_p)
{
    return (struct pool_plugin *) &pool_nop_plugin;
}

static void pool_plugin_nop_dispose(struct pool_plugin *p)
{
    CHECK_PLUGIN("nop", p);
}

struct pool_plugin *pool_plugin_init(struct pool_plugin *p)
{
    if (!p)
        return pool_plugin_nop_create(NULL);
    if (p->flags) {
        /* already inited */
        return p;
    }
    struct pool_plugin *_p = p->Create(p);
    _p->flags |= 1;
    return _p;
}

struct pool_plugin *pool_plugin_clone(struct pool_plugin *p, uint32_t size)
{
    struct pool_plugin *newp = (struct pool_plugin *) malloc(size);
    memcpy(newp, p, size);
    return newp;
}

void pool_process_log(struct pool_plugin *p, struct LogEntry *e)
{
    p->Apply(p, e, 0);
}

void pool_plugin_dispose(struct pool_plugin *p)
{
    if (p->Dispose)
        p->Dispose(p);
}

char *LogEntry_get(struct LogEntry *e, char *key, int klen, int *vlen)
{
    uint16_t i;
    struct Log *log = (struct Log*)&e->data;
    char *data = log_get_data(log);
    for (i = 0; i < log->logsize; ++i) {
        char *next = log_iterator(log, data, i);
        uint16_t len0 = log_get_length(log, i*2+0);
        uint16_t len1 = log_get_length(log, i*2+1);
        if (klen == len0 && strncmp(key, data, klen) == 0) {
            *vlen = len1;
            return data+klen;
        }
        data = next;
    }
    return NULL;
}

typedef struct pool_plugin pool_plugin_t;
DEF_ARRAY_STRUCT0(pool_plugin_t, uint32_t);
DEF_ARRAY_T(pool_plugin_t);
DEF_ARRAY_OP(pool_plugin_t);
struct pool_list {
    konoha_t konoha;
    ARRAY(pool_plugin_t) list;
};

static inline uint64_t TimeMilliSecond(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec;
}

void pool_exec(struct Log *log, int logsize, struct pool_list *plist)
{
    uint32_t size = sizeof(struct LogHead) + logsize;
    char buffer[size];
    struct LogEntry *newe = (struct LogEntry *) buffer;
    newe->h.next = NULL;
    newe->h.size = size;
    newe->h.time = TimeMilliSecond();
    RefInit(newe);
    memcpy(&newe->data, log, logsize);
    struct pool_plugin *p, *pe;
    FOR_EACH_ARRAY(plist->list, p, pe) {
        p->Apply(p, newe, 0);
    }
}

typedef pool_plugin_t *(*fpool_plugin_init)(struct bufferevent *bev);
void pool_add(struct Procedure *q, struct bufferevent *bev, struct pool_list *l)
{
#if 1
    char buf[128] = {};
    memcpy(buf, q->data, q->vlen);
    fprintf(stderr, "procedure: '%s':%d\n", buf, q->vlen);
    memcpy(buf+q->vlen, "_init", 6);
#endif
    //CTX_t _ctx = l->konoha;
    //kMethod *mtd = kKonohaSpace_getMethodNULL(ks, TY_System, MN_("initPlugin"));
    //if (mtd) {
    //    BEGIN_LOCAL(lsfp, K_CALLDELTA + 2);
    //    KSETv(lsfp[K_CALLDELTA+0].o, c->func->self);
    //    KCALL(lsfp, 0, mtd, 0, K_NULL);
    //    END_LOCAL();
    //    pool_plugin_t *plugin = lsfp[0].o->fields[0];
    //    ARRAY_add(pool_plugin_t, &l->list, plugin);
    //}
}

void pool_delete_connection(struct pool_list *l, struct bufferevent *bev)
{
    //TODO
}

extern const kplatform_t* platform_shell(void);
struct pool_list * pool_new(void)
{
    struct pool_list *l = malloc(sizeof(struct pool_list));
    ARRAY_init(pool_plugin_t, &l->list, 4);
    l->konoha = konoha_open(platform_shell());
    return l;
}

void pool_delete(struct pool_list *l)
{
    konoha_close(l->konoha);
    ARRAY_dispose(pool_plugin_t, &l->list);
    free(l);
}

#ifdef __cplusplus
}
#endif
