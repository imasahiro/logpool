#include <stdio.h>
#include "logpool.h"
#include "logpool_internal.h"
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

static void api_fn_flush(logctx ctx, char *buffer, size_t size)
{
    (void)size;
    FILE *fp = cast(flog_t *, ctx->connection)->f.fp;
    fputs(buffer, fp);
}

void *fn_init(logctx ctx, void **args)
{
    char *fname = cast(char *, args[1]);
    flog_t *fl  = cast(flog_t *, logpool::fn_init(ctx, args));
    fl->f.fp = fopen(fname, "w");
    fl->f.fn = api_fn_flush;
    return cast(void *, fl);
}

void fn_flush(logctx ctx, void **args)
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
    file2::fn_init
};
#ifdef __cplusplus
}
#endif
