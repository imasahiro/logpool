#include "pool_plugin/pool_plugin.h"

struct cpu_average {
    int sum;
    int size;
};

static uintptr_t p5_init(uintptr_t context)
{
    struct cpu_average *v = malloc(sizeof(struct cpu_average));
    return (uintptr_t) v;
}

static uintptr_t p5_exit(uintptr_t context)
{
    struct cpu_average *average = (struct cpu_average*) context;
    uintptr_t data = 0;
    if (average->size) {
        data = average->sum/ average->size;
    }
    free(average);
    return data;
}
static uintptr_t p5_func(uintptr_t context, struct LogEntry *e)
{
    struct cpu_average *average = (struct cpu_average*) context;
    int vlen;
    char *val = LogEntry_get(e, "cpu", strlen("cpu"), &vlen);
    char *end = val + vlen;
    if (val) {
        average->sum  += strtol(val, &end, 10);
        average->size += 1;
    }
    return context;
}

static uintptr_t p6_init(uintptr_t context, uint32_t state)
{
    return state;
}

static uintptr_t p6_exit(uintptr_t context)
{
    return 0;
}

static uint16_t p6_write_size(uintptr_t context, uint32_t state, uint16_t *lengths)
{
    lengths[0*2+0] = strlen("TraceID");
    lengths[0*2+1] = strlen("cpu_usage");
    lengths[1*2+0] = strlen("value");
    lengths[1*2+1] = strlen("20");
    return 2;
}

static void p6_write_data(uintptr_t context, struct LogEntry *e, char *buf)
{
    char *base = buf;
#define COPY(buf, S) memcpy(buf, S, strlen(S)); buf += strlen(S)
    COPY(buf, "TraceID");
    COPY(buf, "cpu_usage");
    COPY(buf, "value");
    context = (context > 100) ? 99 : context;
    snprintf(buf, 2, "%d", (int)context);
#undef COPY
}

//static bool val_gt(void *v0, void *v1, uint16_t l0, uint16_t l1)
//{
//    char *e0 = (char*)v0 + l0;
//    char *e1 = (char*)v1 + l1;
//    int d0 = strtol(v0, &e0, 10);
//    int d1 = strtol(v1, &e1, 10);
//    return d0 > d1;
//}

static bool val_eq(void *v0, void *v1, uint16_t l0, uint16_t l1)
{
    return l0 == l1 && memcmp(v0, v1, l0) == 0;
}

struct pool_plugin *cpu_usage_init(struct bufferevent *bev)
{
    struct pool_plugin_print *p0 = POOL_PLUGIN_CLONE(pool_plugin_print);
    struct pool_plugin_val_filter *p1 = POOL_PLUGIN_CLONE(pool_plugin_val_filter);
    struct pool_plugin_copy    *p2 = POOL_PLUGIN_CLONE(pool_plugin_copy);
    struct pool_plugin_react   *p3 = POOL_PLUGIN_CLONE(pool_plugin_react);
    struct pool_plugin_timer   *p4 = POOL_PLUGIN_CLONE(pool_plugin_timer);
    struct pool_plugin_statics *p5 = POOL_PLUGIN_CLONE(pool_plugin_statics);
    struct pool_plugin_create  *p6 = POOL_PLUGIN_CLONE(pool_plugin_create);
    struct pool_plugin_response *p7 = POOL_PLUGIN_CLONE(pool_plugin_response);
    p0->base.apply  = &p1->base;
    p1->base.apply  = &p2->base;
    p2->base.apply  = &p3->base;
    p3->base.apply  = &p4->base;
    p3->base.failed = &p4->base;
    p4->base.apply  = &p5->base;
    p5->base.apply  = &p6->base;
    p6->base.apply  = &p7->base;
    {
        p1->key = "TraceID";
        p1->klen = strlen("TraceID");
        p1->val = "event";
        p1->vlen = strlen("event");
        p1->val_cmp = val_eq;
    }
    {
        p3->conf.traceName = "event";
        p3->conf.key = "tid";
    }
    {
        p4->timer = 100;
        p4->flag_start = 0;
        p4->flag_cont = 1;
        p4->flag_finish = 2;
    }
    {
        p5->finit = p5_init;
        p5->fexit = p5_exit;
        p5->function = p5_func;
    }
    {
        p6->finit = p6_init;
        p6->fexit = p6_exit;
        p6->write_size = p6_write_size;
        p6->write_data = p6_write_data;
    }
    {
        p7->bev = bev;
    }
    return pool_plugin_init((struct pool_plugin *) p0);
}
