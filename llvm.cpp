#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ADT/SmallVector.h>
#include "logpool.h"
#include "lpstring.h"
using namespace llvm;

static Module *global_module = NULL;
extern "C" void logpool_llvm_init(int argc __UNUSED__, char **argv __UNUSED__)
{
    InitializeNativeTarget();
    global_module = new Module("logpool_context", getGlobalContext());
}

namespace logpool {

struct jitctx {
public:
    Module *m_;/*shared*/
    Function *F;
    void *ptr_;
    IRBuilder<> *builder;
    Value *Idx;
    jitctx(Module *m, void *ptr) : m_(m), F(0), ptr_(ptr), builder(0), Idx(0) {}
};

void *fn_init(logctx ctx __UNUSED__, void **args __UNUSED__)
{
    jitctx *buf = new jitctx(global_module, 0);
    return cast(void *, buf);
}

void fn_null(logctx ctx, const char *key, uint64_t v __UNUSED__, sizeinfo_t info __UNUSED__)
{
    IRBuilder<> *builder = cast(jitctx *,ctx->connection)->builder;
    std::string key_(key);
    key_ += "null";
    Value *C = builder->CreateGlobalStringPtr(key_, "v");

}
void fn_bool(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    IRBuilder<> *builder = cast(jitctx *,ctx->connection)->builder;
    Value *C = builder->CreateGlobalStringPtr(key, "k");
}
void fn_int(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    IRBuilder<> *builder = cast(jitctx *,ctx->connection)->builder;
    Value *C = builder->CreateGlobalStringPtr(key, "k");
}
void fn_hex(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    IRBuilder<> *builder = cast(jitctx *,ctx->connection)->builder;
    Value *C = builder->CreateGlobalStringPtr(key, "k");
}
void fn_float(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    IRBuilder<> *builder = cast(jitctx *,ctx->connection)->builder;
    Value *C = builder->CreateGlobalStringPtr(key, "k");
}
void fn_char(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    IRBuilder<> *builder = cast(jitctx *,ctx->connection)->builder;
    Value *C = builder->CreateGlobalStringPtr(key, "k");
}
void fn_string(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    IRBuilder<> *builder = cast(jitctx *,ctx->connection)->builder;
    Value *C = builder->CreateGlobalStringPtr(key, "k");
}
void fn_raw(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    IRBuilder<> *builder = cast(jitctx *,ctx->connection)->builder;
    Value *C = builder->CreateGlobalStringPtr(key, "k");
}
void fn_delim(logctx)
{
    IRBuilder<> *builder = cast(IRBuilder<> *,ctx->connection);
}
void fn_flush(logctx)
{
}

void fn_key_hex(logctx ctx, uint64_t v, uint64_t seq, sizeinfo_t info)
{
    size_t i, size = ctx->logfmt_size;
    jitctx *JIT = static_cast<jitctx*>(ctx->connection);
    char logkey[32];
    //ctx->fn_key(ctx, ctx->logkey.v.u, ctx->logkey.k.seq, ctx->logkey.siz);
    if (JIT->F)
        return;
    LLVMContext &Context = JIT->m_->getContext();
    /* void f(uint64_t seq, ...) */
    Type *Int64Ty   = Type::getInt64Ty(Context);
    Type *Int32Ty   = Type::getInt32Ty(Context);
    Type *Int8PtrTy = Type::getInt8PtrTy(Context);
    struct logfmt *fmt = cast(struct logCtx *, ctx)->fmt;

    SmallVector<Type *, LOGFMT_MAX_SIZE> ArgsTy;
    SmallVector<uint64_t, LOGFMT_MAX_SIZE> Args;
    ArgsTy.push_back(Int64Ty);
    for (i = 0; i < size; ++i, ++fmt) {
        if (fmt->fn == fn_null) {
        }
        else if (fmt->fn == fn_bool ||
                fmt->fn == fn_int ||
                fmt->fn == fn_hex ||
                fmt->fn == fn_float ||
                fmt->fn == fn_char) {
            ArgsTy.push_back(Int64Ty);
        }
        else if (fmt->fn == fn_string ||
                fmt->fn == fn_raw) {
            /* char *, int32_t */
            ArgsTy.push_back(Int8PtrTy);
            ArgsTy.push_back(Int32Ty);
        }
        else {
            assert(0);
        }
        fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
    }

    FunctionType *fnTy = FunctionType::get(Type::getVoidTy(Context), ArgsTy, false);
    Function   *F  = Function::Create(fnTy, GlobalValue::InternalLinkage, "logging", JIT->m_);
    BasicBlock *bb = BasicBlock::Create(Context, "EntryBlock", F);
    IRBuilder<> builder(bb);
    JIT->builder = &builder;
    for (i = 0; i < size; ++i, ++fmt) {
        fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
    }
    cast(struct logCtx *, ctx)->logfmt_size = 0;
    JIT->F = F;
}

void fn_key_string(logctx ctx, uint64_t v, uint64_t seq, sizeinfo_t info)
{
}

} /* namespace logpool */

//struct logapi LLVM_STRING_API = {
//    logpool::fn_null,
//    logpool::fn_bool,
//    logpool::fn_int,
//    logpool::fn_hex,
//    logpool::fn_float,
//    logpool::fn_char,
//    logpool::fn_string,
//    logpool::fn_raw,
//    logpool::fn_delim,
//    logpool::fn_flush,
//    logpool::fn_init,
//};
//
