#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/system_error.h>
#include <llvm/ADT/StringRef.h>
#include <iostream>
#include <assert.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

using namespace llvm;

namespace logpool {

#ifndef LOGPOOL_USE_LLVM_31
#undef HAVE_CLANG
#endif

#ifdef HAVE_CLANG
#include "./llvm_bc.h"
#endif

Module *LoadModule(LLVMContext &Context)
{
#ifdef HAVE_CLANG
    std::string ErrMsg;
    Module *M;
    MemoryBuffer *Buffer = MemoryBuffer::getMemBuffer(
            StringRef((const char *)bitcodes, sizeof(bitcodes)),
            "llvm_bitcode", false);
    assert(Buffer != 0);
    M = ParseBitcodeFile(Buffer, Context, &ErrMsg);
    if (!M) {
        std::cout << ErrMsg << std::endl;
        asm volatile("int3");
    }
    assert(M != 0);
    return M;
#else
    return new Module("logpool_context", Context);
#endif
}
}
