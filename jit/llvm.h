#ifndef LOGPOOL_LLVM_H_
#define LOGPOOL_LLVM_H_
namespace logpool {

typedef void (*flushFn)(logctx, char *buf, size_t len);
struct jitctx_base {
    void *ptr_;
    flushFn fn;
};

void *fn_init(logctx ctx, void **args);
void fn_null(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_bool(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_int(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_hex(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_float(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_char(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_string(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_raw(logctx ctx, const char *key, uint64_t v, sizeinfo_t info);
void fn_delim(logctx ctx);
void fn_flush(logctx ctx, void **fnptr);
} /* namespace logpool */

#endif /* end of include guard */
