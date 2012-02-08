#include <assert.h>
#ifndef LOGPOOL_INTERNAL_H
#define LOGPOOL_INTERNAL_H

#ifndef unlikely
#define unlikely(x)   __builtin_expect(!!(x), 0)
#endif

#ifndef likely
#define likely(x)     __builtin_expect(!!(x), 1)
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

#define __UNUSED__ __attribute__((unused))

#endif /* end of include guard */
