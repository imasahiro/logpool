#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Target/TargetLibraryInfo.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <stdio.h>
#include "logpool.h"
#include "lpstring.h"
#include "jit/llvm.h"

using namespace llvm;

namespace logpool {
static Module *global_module = NULL;
extern Module *LoadModule(LLVMContext &Context);

struct jitctx {
    jitctx_base base;
    Module *m_;/*shared*/
    Function *F;
    IRBuilder<> *builder;
    Value *Str;
    ExecutionEngine *EE;
    int arg_idx;
    int hasEOL;
    jitctx(Module *m, long b) :
        m_(m), F(0),
        builder(0), Str(0), arg_idx(0),
        hasEOL(b) {
            EE = EngineBuilder(m_).setEngineKind(EngineKind::JIT).create();
        }
};

static void api_fn_flush(logctx ctx, char *buffer, size_t size)
{
    (void)ctx;(void)size;
    fputs(buffer, stderr);
}

void *fn_init(logctx ctx __UNUSED__, void **args)
{
    long hasEOL = cast(long, args[0]);
    jitctx *buf = new jitctx(global_module, hasEOL);
    buf->base.fn = api_fn_flush;
    return cast(void *, buf);
}

static void copy_string(jitctx *jit, std::string &Str)
{
    IRBuilder<> *builder = jit->builder;

    size_t Size = Str.size();
    size_t AlignedSize = ALIGN(Size, 8);
    for (size_t i = Size; i < AlignedSize; ++i) {
        Str.push_back('\0');
    }
    Value *L = ConstantInt::get(builder->getInt32Ty(), AlignedSize);
    Value *C = builder->CreateGlobalStringPtr(Str, "v");
    builder->CreateMemCpy(jit->Str, C, L, 1, false);
    jit->Str = builder->CreateConstGEP1_32(jit->Str, Size, "");
}

static void copy_string(jitctx *jit, Value *Str, Value *Len)
{
    IRBuilder<> *builder = jit->builder;
    Type *Int8PtrTy = builder->getInt8PtrTy();
    Str = builder->CreateIntToPtr(Str, Int8PtrTy);
    Len = builder->CreateTrunc(Len, builder->getInt32Ty());

    builder->CreateMemCpy(jit->Str, Str, Len, 1, false);
    jit->Str = builder->CreateGEP(jit->Str, Len);
}

static void copy_number(jitctx *jit, const char *fnName, Value *V)
{
    LLVMContext &Context = jit->m_->getContext();
    IRBuilder<> *builder = jit->builder;
    Type *Int8PtrTy = Type::getInt8PtrTy(Context);
    Type* ArgsTy[] = {
        jit->Str->getType(),
        V->getType(),
    };
    FunctionType *fnTy = FunctionType::get(Int8PtrTy, ArgsTy, false);
    Function *F = cast<Function>(jit->m_->getOrInsertFunction(fnName, fnTy));
    jit->Str = builder->CreateCall2(F, jit->Str, V);
}

static Value *get_arg(jitctx *jit)
{
    IRBuilder<> *builder = jit->builder;
    Value *Arg = ++(jit->F->arg_begin());
    Arg = builder->CreateConstInBoundsGEP1_32(Arg, jit->arg_idx++);
    Arg = builder->CreateLoad(Arg);
    return Arg;
}

static void copy_string(jitctx *jit, const char *text)
{
    std::string Str(text);
    copy_string(jit, Str);
}

static void copy_char(jitctx *jit, Value *C)
{
    IRBuilder<> *builder = jit->builder;
    StoreInst *SI = builder->CreateStore(C, jit->Str);
    SI->setAlignment(1);
    jit->Str = builder->CreateConstGEP1_32(jit->Str, 1, "");
}

static void copy_char(jitctx *jit, char c)
{
    Value *C = jit->builder->getInt8(c);
    copy_char(jit, C);
}

void fn_null(logctx ctx, const char *key, uint64_t v __UNUSED__, sizeinfo_t info __UNUSED__)
{
    jitctx *jit = cast(jitctx *, ctx->connection);
    std::string key_(key);
    key_ += "null";
    copy_string(jit, key_);
}
void fn_bool(logctx ctx, const char *key, uint64_t v __UNUSED__, sizeinfo_t info __UNUSED__)
{
    jitctx *jit = cast(jitctx *, ctx->connection);
    IRBuilder<> *builder = jit->builder;
    static const char T_[] = "true";
    static const char F_[] = "false";
    Value *Arg = get_arg(jit);
    Type  *ArgTy = Arg->getType();

    copy_string(jit, key);
    Value *T = builder->CreateGlobalStringPtr(T_, "T");
    Value *F = builder->CreateGlobalStringPtr(F_, "F");
    Value *TLen = ConstantInt::get(ArgTy, strlen(T_));
    Value *FLen = ConstantInt::get(ArgTy, strlen(F_));

    Value *Str, *Len;
    Arg = builder->CreateICmpEQ(Arg, ConstantInt::get(ArgTy, 0));
    Str = builder->CreateSelect(Arg, T, F);
    Len = builder->CreateSelect(Arg, TLen, FLen);
    copy_string(jit, Str, Len);
}
void fn_int(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    (void)v;(void)info;
    jitctx *jit = cast(jitctx *, ctx->connection);
    copy_string(jit, key);
    copy_number(jit, "llvm_put_i", get_arg(jit));
}
void fn_hex(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    (void)v;(void)info;
    jitctx *jit = cast(jitctx *, ctx->connection);
    std::string key_(key);
    key_ += "0x";
    copy_string(jit, key_);
    copy_number(jit, "llvm_put_h", get_arg(jit));
}
void fn_float(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    (void)v;(void)info;
    jitctx *jit = cast(jitctx *, ctx->connection);
    IRBuilder<> *builder = jit->builder;
    Type *DoubleTy = builder->getDoubleTy();
    Value *V;

    copy_string(jit, key);
    V = get_arg(jit);
    V = builder->CreateBitCast(V, DoubleTy);
    copy_number(jit, "llvm_put_f", V);

}
void fn_char(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    (void)v;(void)info;
    jitctx *jit = cast(jitctx *, ctx->connection);
    copy_string(jit, key);
    copy_char(jit, get_arg(jit));
}

void fn_string(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    (void)v;(void)info;
    jitctx *jit = cast(jitctx *, ctx->connection);
    std::string key_(key);
    key_ += "'";
    copy_string(jit, key_);
    Value *Str = get_arg(jit);
    Value *Len = get_arg(jit);
    copy_string(jit, Str, Len);
    copy_char(jit, '\'');
}
void fn_raw(logctx ctx, const char *key, uint64_t v, sizeinfo_t info)
{
    (void)v;(void)info;
    jitctx *jit = cast(jitctx *, ctx->connection);
    copy_string(jit, key);
}

void fn_delim(logctx ctx)
{
    jitctx *jit = cast(jitctx *, ctx->connection);
    copy_char(jit, ',');
}
static void *emit_code(logctx ctx)
{
    size_t i, size = ctx->logfmt_size;
    jitctx *JIT = static_cast<jitctx*>(ctx->connection);
    Module *M = JIT->m_;
    LLVMContext &Context = M->getContext();
    /* void f(uint64_t seq, ...) */
    Type *Int8PtrTy  = Type::getInt8PtrTy(Context);
    Type *Int64PtrTy = Type::getInt64PtrTy(Context);
    struct logfmt *fmt = cast(struct logCtx *, ctx)->fmt;

    Type * ArgsTy[] = {
        Int8PtrTy,
        Int64PtrTy,
    };
    FunctionType *fnTy = FunctionType::get(Type::getVoidTy(Context), ArgsTy, false);
    Function *F = Function::Create(fnTy,
            GlobalValue::ExternalLinkage, "logging", M);
    Function::arg_iterator I = F->arg_begin();
    JIT->Str = I;
    (*I).setName("Buf");++I;
    (*I).setName("Arg");
    BasicBlock *bb = BasicBlock::Create(Context, "EntryBlock", F);
    IRBuilder<> builder(bb);
    JIT->builder = &builder;
    JIT->F = F;
    fmt = cast(struct logCtx *, ctx)->fmt;

    if (size) {
        fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
        fmt++;
        for (i = 1; i < size; ++i, ++fmt) {
            ctx->formatter->fn_delim(ctx);
            fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
        }
    }
    if (JIT->hasEOL) {
        copy_char(JIT, '\n');
    }
    copy_char(JIT, '\0');

    builder.CreateRetVoid();

    PassManager PM;
    PassManagerBuilder Builder;
    Builder.OptLevel = 3;
    Builder.SizeLevel = 2;
    Builder.LibraryInfo = new TargetLibraryInfo(Triple(M->getTargetTriple()));
    Builder.DisableUnitAtATime = false;
    Builder.DisableUnrollLoops = false;
    Builder.DisableSimplifyLibCalls = false;
    Builder.Inliner = createFunctionInliningPass(225);
    PM.add(new TargetData(*(JIT->EE->getTargetData())));
    PM.add(createVerifierPass());
    Builder.populateLTOPassManager(PM, true, true);
    PM.run(*M);

    //(*M).dump();
    void *fnptr = JIT->EE->getPointerToFunction(F);
    JIT->F = 0;
    JIT->builder = 0;
    return fnptr;
}

static int push_args(uint64_t *a, int argc, struct logfmt *fmt)
{
    a[argc] = fmt->v.u;
    if (fmt->fn == fn_string || fmt->fn == fn_raw) {
        a[++argc] = get_l1(fmt->siz);
    }
    return argc + 1;
}

typedef void (*jitFn)(char *, uint64_t *);

void fn_flush(logctx ctx, void **fnptr __UNUSED__)
{
    char buffer[256], *p = buffer;

    ctx->fn_key(cast(logctx, &p), ctx->logkey.v.u, ctx->logkey.k.seq, ctx->logkey.siz);
    p[0] = ',';
    if (ctx->logfmt_size) {
        uint64_t params[LOGFMT_MAX_SIZE*2];
        jitctx *JIT = static_cast<jitctx*>(ctx->connection);
        if (!*fnptr)
            *fnptr = emit_code(ctx);
        jitFn F = reinterpret_cast<jitFn>(*fnptr);
        struct logfmt *fmt = cast(struct logCtx *, ctx)->fmt;
        size_t argc = 0, size = ctx->logfmt_size;
        for (size_t i = 0; i < size; ++i, ++fmt) {
            argc = push_args(params, argc, fmt);
        }
        cast(struct logCtx *, ctx)->logfmt_size = 0;
        F(p+1, params);
        JIT->base.fn(ctx, buffer, 0);
    }
    ++(cast(struct logCtx *, ctx)->logkey.k.seq);
}

static char *put_seq(char *buf, uint64_t seq)
{
    buf[0] = '+';
    buf = put_hex(buf, seq);
    return buf;
}

void fn_key_hex(char **bufp, uint64_t v, uint64_t seq, sizeinfo_t info __UNUSED__)
{
    char *buf = *bufp;
    put_char2(buf, '0', 'x');
    buf = put_hex(buf+2, v);
    *bufp = put_seq(buf, seq);
}

void fn_key_string(char **bufp, uint64_t v, uint64_t seq, sizeinfo_t info)
{
    char *buf = *bufp;
    char *s = cast(char *, v);
    buf = put_string(buf, s, get_l2(info));
    *bufp = put_seq(buf, seq);
}

} /* namespace logpool */

extern "C" char *llvm_put_h(char *p, uintptr_t value)
{
    return put_hex(p, value);
}

extern "C" char *llvm_put_i(char *p, intptr_t value)
{
    return put_i(p, value);
}

extern "C" char *llvm_put_f(char *p, double f)
{
    return put_f(p, f);
}

static struct keyapi LLVM_KEY_API = {
    (keyFn)logpool::fn_key_hex,
    (keyFn)logpool::fn_key_string
};

#ifdef LOGPOOL_USE_LLVM_31
static const char* GetHostTriple() {
#ifdef LLVM_HOSTTRIPLE
  return LLVM_HOSTTRIPLE;
#else
  return LLVM_DEFAULT_TARGET_TRIPLE;
#endif
}
#endif

extern "C" struct keyapi *logpool_llvm_api_init(void)
{
    InitializeNativeTarget();
    Module *M = logpool::LoadModule(getGlobalContext());
#ifdef LOGPOOL_USE_LLVM_31
    std::string Error;
    const char *Triple = GetHostTriple();
    const Target* T(TargetRegistry::lookupTarget(Triple, Error));
    TargetOptions options;
    //options.NoFramePointerElim = true;
#ifdef DEBUG_MODE
    options.JITEmitDebugInfo = true;
#endif
    TargetMachine    *TM = T->createTargetMachine(Triple, "", "", options);
    const TargetData *TD = TM->getTargetData();
    M->setDataLayout(TD->getStringRepresentation());
    M->setTargetTriple(TM->getTargetTriple());
#endif
    logpool::global_module = M;
    return &LLVM_KEY_API;
}

struct logapi LLVM_STRING_API = {
    logpool::fn_null,
    logpool::fn_bool,
    logpool::fn_int,
    logpool::fn_hex,
    logpool::fn_float,
    logpool::fn_char,
    logpool::fn_string,
    logpool::fn_raw,
    logpool::fn_delim,
    logpool::fn_flush,
    logpool::fn_init
};
