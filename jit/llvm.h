#include "logpool_internal.h"

#ifndef LOGPOOL_LLVM_H_
#define LOGPOOL_LLVM_H_
namespace logpool {

typedef void (*flushFn)(logctx_t *, char *buf, size_t len);
struct jitctx_base {
    void *ptr_;
    flushFn fn;
};

void *fn_init(logctx_t *ctx, logpool_param_t *);
void fn_close(logctx_t *ctx);
void fn_null(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_bool(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_int(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_hex(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_float(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_char(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_string(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_raw(logctx_t *ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_delim(logctx_t *ctx);
void fn_flush(logctx_t *ctx, void **fnptr);
} /* namespace logpool */

#endif /* end of include guard */
