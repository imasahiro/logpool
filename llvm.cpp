#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Intrinsics.h>
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
//void f() {
//    std::vector<Type*> args_type;
//    args_type.push_back(floatTy);
//    BasicBlock *bb = BasicBlock::Create(Context, "EntryBlock", func);
//    IRBuilder<> builder(bb);
//    Value *v = ConstantFP::get(floatTy, -10.0);
//    v = builder->CreateCall(f, v);
//    builder->CreateRet(v);
//    std::cout << "before" << std::endl;
//    (*m).dump();
//    PassManager mpm;
//    mpm.add(createIPSCCPPass());
//    mpm.add(createFunctionInliningPass());
//    mpm.add(createLICMPass());
//    mpm.add(createGVNPass());
//    mpm.add(createGlobalDCEPass());
//    mpm.run(*m);
//    std::cout << std::endl << "before" << std::endl;
//    (*m).dump();
//}

namespace logpool {
struct jitctx {
    void *ptr_;
    Module *m_;/*shared*/
    Function *F;
    IRBuilder<> *builder;
    Value *Str;
    ExecutionEngine *EE;
    int arg_idx;
    jitctx(Module *m, void *ptr) :
        ptr_(ptr), m_(m), F(0),
        builder(0), Str(0), arg_idx(0) {
            EE = EngineBuilder(m_).setEngineKind(EngineKind::JIT).create();
        }
};

void *fn_init(logctx ctx __UNUSED__, void **args __UNUSED__)
{
    jitctx *buf = new jitctx(global_module, 0);
    return cast(void *, buf);
}

static Function *CreateMemCpy(Module *M, Value *Dst, Value *Src)
{
    LLVMContext &Context = M->getContext();
    //declare void @llvm.memcpy.p0i8.p0i8.i32(i8* <dest>, i8* <src>,
    //        i32 <len>, i32 <align>, i1 <isvolatile>)
    Type* List[] = {
        Dst->getType(),
        Src->getType(),
        Type::getInt32Ty(Context),
        Type::getInt32Ty(Context),
        Type::getInt1Ty(Context)
    };
    return Intrinsic::getDeclaration(M, Intrinsic::memcpy, List);
}
static void copy_string(jitctx *jit, std::string &Str)
{
    IRBuilder<> *builder = jit->builder;
    Value *C = builder->CreateGlobalStringPtr(Str, "v");
    Function *F = CreateMemCpy(jit->m_, jit->Str, C);
    Value *Length = builder->getInt32(Str.size());
    builder->CreateCall5(F, jit->Str, C, Length,
            builder->getInt32(8), builder->getInt1(0));
    jit->Str = builder->CreateConstGEP1_32(jit->Str, Str.size(), "");
}

static void copy_string(jitctx *jit, Value *Str, Value *Len)
{
    IRBuilder<> *builder = jit->builder;
    Type *Int8PtrTy = builder->getInt8PtrTy();
    Str = builder->CreateIntToPtr(Str, Int8PtrTy);

    Function *F = CreateMemCpy(jit->m_, jit->Str, Str);
    builder->CreateCall5(F, jit->Str, Str, Len,
            builder->getInt32(8), builder->getInt1(0));
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
    copy_string(jit, key);
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
    copy_string(jit, key);
    Value *Str = get_arg(jit);
    Value *Len = get_arg(jit);
    copy_string(jit, Str, Len);
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
    LLVMContext &Context = JIT->m_->getContext();
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
            GlobalValue::InternalLinkage, "logging", JIT->m_);
    Function::arg_iterator I = F->arg_begin();
    JIT->Str = I;
    (*I).setName("Buf");++I;
    (*I).setName("Arg");
    BasicBlock *bb = BasicBlock::Create(Context, "EntryBlock", F);
    IRBuilder<> builder(bb);
    JIT->builder = &builder;
    JIT->F   = F;
    fmt = cast(struct logCtx *, ctx)->fmt;

    if (size) {
        ctx->formatter->fn_delim(ctx);
        fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
        fmt++;
        for (i = 1; i < size; ++i, ++fmt) {
            ctx->formatter->fn_delim(ctx);
            fmt->fn(ctx, fmt->k.key, fmt->v.u, fmt->siz);
        }
    }
    copy_char(JIT, '\0');
    builder.CreateRetVoid();
    (*JIT->m_).dump();
    void *fnptr = JIT->EE->getPointerToFunction(JIT->F);
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
    uint64_t params[LOGFMT_MAX_SIZE*2];
    if (!*fnptr) {
        *fnptr = emit_code(ctx);
    }
    jitFn F = reinterpret_cast<jitFn>(*fnptr);
    ctx->fn_key(cast(logctx, &p), ctx->logkey.v.u, ctx->logkey.k.seq, ctx->logkey.siz);
    if (ctx->logfmt_size) {
        struct logfmt *fmt = cast(struct logCtx *, ctx)->fmt;
        size_t argc = 0, size = ctx->logfmt_size;
        for (size_t i = 0; i < size; ++i, ++fmt) {
            argc = push_args(params, argc, fmt);
        }
        cast(struct logCtx *, ctx)->logfmt_size = 0;
    }
    F(p, params);
    fprintf(stderr, "%s\n", buffer);
    ++(cast(struct logCtx *, ctx)->logkey.k.seq);
}

static void reverse(char *const start, char *const end, const int len)
{
    int i, l = len / 2;
    register char *s = start;
    register char *e = end - 1;
    for (i = 0; i < l; i++) {
        char tmp = *s;
        tmp  = *s;
        *s++ = *e;
        *e-- = tmp;
    }
}

template<int radix>
static inline char *put_d(char *const p, uint64_t uvalue)
{
    int i = 0;
    static const char _t_[] = "0123456789abcdef";
    while (uvalue != 0) {
        int r = uvalue % radix;
        p[i]  = _t_[r];
        uvalue /= radix;
        i++;
    }
    reverse(p, p + i, i);
    return p + i;
}

static char *put_seq(char *buf, uint64_t seq)
{
    uintptr_t seq_ = seq / 16, r = seq % 16;
    buf[0] = '+';
    buf = put_d<16>(buf, seq_);
    buf[0] = '0' + r;
    return buf + 1;
}

static char *put_string(char *p, const char *s, short size)
{
    int i;
    for (i = 0; i < size; ++i) {
        p[i] = s[i];
    }
    return p + i;
}

void fn_key_hex(char **bufp, uint64_t v, uint64_t seq, sizeinfo_t info __UNUSED__)
{
    char *buf = *bufp;
    buf[0] = '0';
    buf[0] = 'x';
    buf = put_d<16>(buf, v);
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
    return logpool::put_d<16>(p, value);
}

extern "C" char *llvm_put_i(char *p, intptr_t value)
{
    if(value < 0) {
        p[0] = '-'; p++;
        value = -value;
    }
    uintptr_t u = value / 10, r = value % 10;
    if (u != 0) {
        p = logpool::put_d<10>(p, u);
    }
    p[0] = ('0' + r);
    return p + 1;
}

extern "C" char *llvm_put_f(char *p, double f)
{
    intptr_t value = (intptr_t) (f*1000);
    if(value < 0) {
        p[0] = '-'; p++;
        value = -value;
    }
    intptr_t u = value / 1000, r = value % 1000;
    if(u != 0) {
        p = logpool::put_d<10>(p, u);
    }
    else {
        p[0] = '0'; p++;
    }
    p[0] = '.'; p++;
    u = r / 100;
    r = r % 100;
    p[0] = ('0' + (u)); p++;
    p[0] = ('0' + (r / 10)); p++;
    p[0] = ('0' + (r % 10));
    return p + 1;
}

static struct keyapi LLVM_KEY_API = {
    (keyFn)logpool::fn_key_hex,
    (keyFn)logpool::fn_key_string
};

extern "C" struct keyapi *logpool_llvm_api_init(void)
{
    InitializeNativeTarget();
    global_module = new Module("logpool_context", getGlobalContext());
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
    logpool::fn_init,
};
