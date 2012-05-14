#include <assert.h>
#ifndef LOGPOOL_INTERNAL_H
#define LOGPOOL_INTERNAL_H

#ifndef unlikely
#define unlikely(x)   __builtin_expect(!!(x), 0)
#endif

#ifndef likely
#define likely(x)     __builtin_expect(!!(x), 1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct keyapi {
    keyFn hex;
    keyFn str;
};

static inline double u2f(uint64_t u)
{
    union {uint64_t u; double f;} v;
    v.u = u;
    return v.f;
}

struct logpool_plugin_header {
    void *param0;
    void *param1;
};

typedef void logplugin_t;

#define __UNUSED__ __attribute__((unused))

__UNUSED__ static int logpool_default_priority(logpool_t *ctx __UNUSED__, int priority __UNUSED__)
{
    return 1;
}

static inline void logpool_context_switch(logpool_t *ctx, void *connection)
{
    cast(struct logpool *, ctx)->connection = connection;
}

void logpool_format_flush(logpool_t *ctx);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard */
