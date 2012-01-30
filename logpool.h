#ifndef LOGPOOL_H_
#define LOGPOOL_H_

#include <stdint.h>
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOGFMT_MAX_SIZE 8

struct logCtx;
struct ltrace;
struct lstate;
typedef long sizeinfo_t;
typedef const struct logCtx *logctx;
typedef const struct ltrace ltrace_t;
typedef const struct lstate lstate_t;
typedef struct logapi logapi_t;
typedef struct logfmt logfmt_t;
typedef void  (*logFn)(logctx, const char *K, uint64_t v, sizeinfo_t info);
typedef char *(*keyFn)(logctx, uint64_t v, uint64_t seq, sizeinfo_t size);

struct logfmt {
    logFn fn;
    union key {
        uint64_t seq;
        const char *key;
    } k;
    union logdata {
        uint64_t u;
        double f;
        char *s;
    } v;
    sizeinfo_t siz;
};

struct logapi {
    logFn fn_null;
    logFn fn_bool;
    logFn fn_int;
    logFn fn_hex;
    logFn fn_float;
    logFn fn_char;
    logFn fn_string;
    logFn fn_raw;
    void  (*fn_delim)(logctx);
    void  (*fn_flush)(logctx, void**);
    void *(*fn_init)(logctx, void**);
    void  (*fn_close)(logctx);
};

enum LOGPOOL_EXEC_MODE {
    LOGPOOL_DEFAULT,
    LOGPOOL_JIT
};

void logpool_init(enum LOGPOOL_EXEC_MODE mode);
void logpool_exit(void);

struct logCtx {
    void *connection;
    keyFn     fn_key;
    logapi_t *formatter;
    logfmt_t logkey;
    logfmt_t fmt[LOGFMT_MAX_SIZE];
    long logfmt_size;
};

struct ltrace {
    struct logCtx ctx;
    ltrace_t *parent;
};

ltrace_t *ltrace_open(ltrace_t *parent, struct logapi *api, void **param);
ltrace_t *ltrace_open_syslog(ltrace_t *parent);
ltrace_t *ltrace_open_file(ltrace_t *parent, char *filename);
ltrace_t *ltrace_open_memcache(ltrace_t *parent, char *host, long ip);
void ltrace_close(ltrace_t *p);

/* lstate API */
struct lstate {
    struct logCtx ctx;
    uint64_t state;
};

lstate_t *lstate_open(const char *state_name, struct logapi *api, void **param);
lstate_t *lstate_open_syslog(const char *state);
lstate_t *lstate_open_file(const char *state, char *filename);
lstate_t *lstate_open_memcache(const char *state, char *host, long ip);
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

void logctx_init(logctx ctx, struct logapi *api, void **param);
void logctx_format_flush(logctx ctx);
void logctx_append_fmtdata(logctx ctx, const char *key, uint64_t v, logFn f, sizeinfo_t info);
void logctx_init_logkey(logctx ctx, uint64_t v, sizeinfo_t siz);

#define __UNUSED__ __attribute__((unused))
#define cast(T, V) ((T)(V))

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

#define LCTX(V) (cast(logctx, V))
#define ltrace_record(T, E, ...) do {\
    static void *__LOGDATA__ = NULL;\
    logctx __CTX__ = cast(logctx, T);\
    logctx_init_logkey(__CTX__, cast(uint64_t, E), sizeinfo_create(0, strlen(E)));\
    __VA_ARGS__;\
} while (0)

#define lstate_record(R, ...) do {\
    static void *__LOGDATA__ = NULL;\
    logctx __CTX__ = cast(logctx, R);\
    logctx_init_logkey(__CTX__, R->state, 0);\
    __VA_ARGS__;\
} while (0)

static inline sizeinfo_t sizeinfo_create(short s1, short s2)
{
    return s1 << (sizeof(short) * 8) | s2;
}

#define LOG_END       __CTX__->formatter->fn_flush(__CTX__, &__LOGDATA__);
#define LOG_s(K,V)    __extension__(({\
        static const char __K__[] = K ":";\
        logctx_append_fmtdata(__CTX__, __K__, cast(uint64_t, V),\
            __CTX__->formatter->fn_string,\
            sizeinfo_create(strlen(V), strlen(__K__)));}))
#define LOG_i(K,V)    __extension__(({\
        static const char __K__[] = K ":";\
        logctx_append_fmtdata(__CTX__, __K__, cast(uint64_t, V),\
            __CTX__->formatter->fn_int, sizeinfo_create(0, strlen(__K__)));}))
#define LOG_f(K,V)    __extension__(({\
        static const char __K__[] = K ":";\
        logctx_append_fmtdata(__CTX__, __K__, f2u(V),\
            __CTX__->formatter->fn_float, sizeinfo_create(0, strlen(__K__)));}))
#define LOG_p(K,V)    __extension__(({\
        static const char __K__[] = K ":";\
        logctx_append_fmtdata(__CTX__, __K__, cast(uint64_t, V),\
            __CTX__->formatter->fn_hex, sizeinfo_create(0, strlen(__K__)));}))

#ifdef __cplusplus
}
#endif

#endif /* end of include guard */
