#ifndef LOGPOOL_H_
#define LOGPOOL_H_
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define __UNUSED__ __attribute__((unused))
#define cast(T, V) ((T)(V))
struct logctx;
struct ltrace;
struct lstate;
struct logapi;
typedef const struct logctx * logctx;
typedef void (*logFn)(logctx, const char *K, uint64_t v);
typedef const struct ltrace ltrace_t;
typedef const struct lstate lstate_t;
typedef const void ldata_t;
typedef struct logapi logapi_t;
#define LOGFMT_MAX_SIZE 8
struct logctx {
    void *connection;
    struct logapi {
        logFn fn_null;
        logFn fn_bool;
        logFn fn_int;
        logFn fn_hex;
        logFn fn_float;
        logFn fn_char;
        logFn fn_string;
        logFn fn_raw;
        void   (*fn_delim)(logctx);
        void   (*fn_flush)(logctx);
        void  *(*fn_init)(logctx, void*);
    } *formatter;
    struct logfmt {
        logFn fn;
        const char *key;
        union logdata {
            uint64_t u;
            double f;
            char *s;
        } v;
    } fmt[LOGFMT_MAX_SIZE];
    long logfmt_size;
};

struct ltrace {
    struct logctx ctx;
    ltrace_t *parent;
};

struct lstate {
    struct logctx ctx;
    uint64_t state;
};

ltrace_t *ltrace_open(ltrace_t *parent, logapi_t *);
void ltrace_record(ltrace_t *p, const char* event, ldata_t *);
void ltrace_close(ltrace_t *p);

lstate_t *lstate_open(const char *state_name, logapi_t *);
void lstate_record(lstate_t *p, const ldata_t*);
void lstate_close(lstate_t *p);

static inline uint64_t f2u(double f)
{
    union {uint64_t u; double f;} v;
    v.f = f;
    return v.u;
}

static inline double u2f(uint64_t u)
{
    union {uint64_t u; double f;} v;
    v.u = u;
    return v.f;
}

void logctx_init(logctx ctx, struct logapi *api, void *param);
void logctx_format_flush(logctx ctx);
void logctx_append_fmtdata(logctx ctx, const char *key, uint64_t v, logFn f);
static inline void logctx_fmt_start(logctx ctx, uint64_t v, logFn f)
{
    logctx_append_fmtdata(ctx, "key", v, f);
}

#define LCTX(V) (cast(logctx, V))
#define ltrace_record(T, E, ...) do {\
    logctx __CTX__ = cast(logctx, T);\
    logctx_fmt_start(__CTX__, cast(uint64_t, E), __CTX__->formatter->fn_string);\
    __VA_ARGS__;\
} while (0)

#define lstate_record(R, ...) do {\
    logctx __CTX__ = cast(logctx, R);\
    logctx_fmt_start(__CTX__, R->state, __CTX__->formatter->fn_hex);\
    __VA_ARGS__;\
} while (0)

#define LOG_END       __CTX__->formatter->fn_flush(__CTX__);
#define LOG_s(K,V)    ({logctx_append_fmtdata(__CTX__, K, cast(uint64_t, V), __CTX__->formatter->fn_string);})
#define LOG_i(K,V)    ({logctx_append_fmtdata(__CTX__, K, V, __CTX__->formatter->fn_int);})
#define LOG_f(K,V)    ({logctx_append_fmtdata(__CTX__, K, f2u(V), __CTX__->formatter->fn_float);})
#define LOG_p(K,V)    ({logctx_append_fmtdata(__CTX__, K, cast(uint64_t, V), __CTX__->formatter->fn_hex);})

#endif /* end of include guard */
