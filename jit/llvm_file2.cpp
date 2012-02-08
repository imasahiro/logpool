#include <stdio.h>
#include "logpool.h"
#include "lpstring.h"
#include "jit/llvm.h"

namespace file2 {
using namespace logpool;

typedef union fileinfo {
    struct jitctx_base base;
    struct _f {
        FILE *fp;
        logpool::flushFn fn;
    } f;
} flog_t;

static void api_fn_flush(logctx_t *ctx, char *buffer, size_t size)
{
    (void)size;
    FILE *fp = cast(flog_t *, ctx->connection)->f.fp;
    fputs(buffer, fp);
}

void *fn_init(logctx_t *ctx, logpool_param_t *p)
{
    struct logpool_param_file *args = cast(struct logpool_param_file *, p);
    char *fname = cast(char *, args->fname);
    flog_t *fl  = cast(flog_t *, logpool::fn_init(ctx, p));
    fl->f.fp = fopen(fname, "w");
    fl->f.fn = api_fn_flush;
    return cast(void *, fl);
}

void fn_flush(logctx_t *ctx, void **args)
{
    logpool::fn_flush(ctx, args);
}

} /* namespace file2 */

#ifdef __cplusplus
extern "C" {
#endif

struct logapi LLVM_FILE2_API = {
    logpool::fn_null,
    logpool::fn_bool,
    logpool::fn_int,
    logpool::fn_hex,
    logpool::fn_float,
    logpool::fn_char,
    logpool::fn_string,
    logpool::fn_raw,
    logpool::fn_delim,
    file2::fn_flush,
    file2::fn_init,
    logpool::fn_close
};
#ifdef __cplusplus
}
#endif
