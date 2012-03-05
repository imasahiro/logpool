#include <stdint.h>
#include <string.h>
#include <syslog.h> /* LOG_EMERG etc.. */

#ifndef LOGPOOL_H_
#define LOGPOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

struct logctx;
struct ltrace;
typedef long sizeinfo_t;
typedef const struct logctx logctx_t;
typedef const struct ltrace ltrace_t;
typedef struct logapi logapi_t;
typedef struct logfmt logfmt_t;

#ifndef LOG_EMERG
#define LOG_EMERG   0 /* system is unusable */
#define LOG_ALERT   1 /* action must be taken immediately */
#define LOG_CRIT    2 /* critical conditions */
#define LOG_ERR     3 /* error conditions */
#define LOG_WARNING 4 /* warning conditions */
#define LOG_NOTICE  5 /* normal but significant condition */
#define LOG_INFO    6 /* informational */
#define LOG_DEBUG   7 /* debug-level messages */
#endif

/* param format */
typedef struct logpool_param {
    int logfmt_capacity;
} logpool_param_t;

struct logpool_param_string {
    int logfmt_capacity;
    uintptr_t buffer_size;
};

struct logpool_param_syslog {
    int logfmt_capacity;
    uintptr_t buffer_size;
};

struct logpool_param_file {
    int logfmt_capacity;
    uintptr_t buffer_size;
    const char *fname;
};

struct logpool_param_memcache {
    int logfmt_capacity;
    uintptr_t buffer_size;
    const char *host;
    long port;
};

struct logpool_param_filter {
    int logfmt_capacity;
    int priority;
    logapi_t *api;
    struct logpool_param *param;
};

#define LOGPOOL_MULTIPLEXER_MAX 4
struct logpool_param_multiplexer {
    int logfmt_capacity;
    int argc;
    struct plugin_param {
        logapi_t *api;
        struct logpool_param *param;
    } args[LOGPOOL_MULTIPLEXER_MAX];
};

/* log formatter API */
typedef void  (*logFn)(logctx_t *, const char *K, uint64_t v, sizeinfo_t info);
typedef char *(*keyFn)(logctx_t *, uint64_t v, uint64_t seq, sizeinfo_t size);

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
    void  (*fn_delim)(logctx_t *);
    void  (*fn_flush)(logctx_t *, void**);
    void *(*fn_init) (logctx_t *, logpool_param_t *);
    void  (*fn_close)(logctx_t *);
    int   (*fn_priority)(logctx_t *, int);
};

enum LOGPOOL_EXEC_MODE {
    LOGPOOL_DEFAULT,
    LOGPOOL_JIT
};

void logpool_init(enum LOGPOOL_EXEC_MODE mode);
void logpool_exit(void);

struct logctx {
    void *connection;
    keyFn     fn_key;
    logapi_t *formatter;
    logfmt_t logkey;
    logfmt_t *fmt;
    long logfmt_size;
    long logfmt_capacity;
    uintptr_t is_flushed;
};

/* ltrace API */
struct ltrace {
    struct logctx ctx;
    ltrace_t *parent;
};

ltrace_t *ltrace_open(ltrace_t *parent, struct logapi *api, logpool_param_t *);
ltrace_t *ltrace_open_syslog(ltrace_t *parent);
ltrace_t *ltrace_open_file(ltrace_t *parent, char *filename);
ltrace_t *ltrace_open_memcache(ltrace_t *parent, char *host, long ip);
void ltrace_close(ltrace_t *p);

void logctx_flush(logctx_t *ctx, void **args);
void logctx_append_fmtdata(logctx_t *ctx, const char *key, uint64_t v, logFn f, sizeinfo_t info);
int  logctx_init_logkey(logctx_t *ctx, int priority, uint64_t v, sizeinfo_t siz);

#define cast(T, V) ((T)(V))

#define ltrace_record(T, PRIORITY, E, ...) do {\
    static void *__LOGDATA__ = NULL;\
    logctx_t *__CTX__ = cast(logctx_t *, T);\
    if (logctx_init_logkey(__CTX__, PRIORITY, cast(uint64_t, E), build_sizeinfo(0, strlen(E)))) {\
        __VA_ARGS__;\
    }\
} while (0)

#define LOG_END       logctx_flush(__CTX__, &__LOGDATA__);
#define LOG_s(K,V)    __extension__(({\
        static const char __K__[] = K ":";\
        logctx_append_fmtdata(__CTX__, __K__, cast(uint64_t, V),\
            __CTX__->formatter->fn_string,\
            build_sizeinfo(strlen(V), strlen(__K__)));}))
#define LOG_i(K,V)    __extension__(({\
        static const char __K__[] = K ":";\
        logctx_append_fmtdata(__CTX__, __K__, cast(uint64_t, V),\
            __CTX__->formatter->fn_int, build_sizeinfo(0, strlen(__K__)));}))
#define LOG_f(K,V)    __extension__(({\
        static const char __K__[] = K ":";\
        logctx_append_fmtdata(__CTX__, __K__, f2u(V),\
            __CTX__->formatter->fn_float, build_sizeinfo(0, strlen(__K__)));}))
#define LOG_p(K,V)    __extension__(({\
        static const char __K__[] = K ":";\
        logctx_append_fmtdata(__CTX__, __K__, cast(uint64_t, V),\
            __CTX__->formatter->fn_hex, build_sizeinfo(0, strlen(__K__)));}))

/* inline functions */
static inline sizeinfo_t build_sizeinfo(short s1, short s2)
{
    return s1 << (sizeof(short) * 8) | s2;
}

static inline uint64_t f2u(double f)
{
    union {uint64_t u; double f;} v;
    v.f = f;
    return v.u;
}

#ifdef __cplusplus
}
#endif

#endif /* end of include guard */
