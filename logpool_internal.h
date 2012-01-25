#ifndef LOGCTX_H_
#define LOGCTX_H_

#ifndef LOGPOOL_H_
#error Include "logpool.h" before including this file
#endif

#ifdef __cplusplus
extern "C" {
#endif

void logctx_init(logctx ctx, struct logapi *api, void **param);

static inline double u2f(uint64_t u)
{
    union {uint64_t u; double f;} v = {0};
    v.u = u;
    return v.f;
}

#ifdef __cplusplus
}
#endif

#endif /* end of include guard */
