#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/system_error.h>
#include <llvm/ADT/StringRef.h>
#include <iostream>
#include <assert.h>


using namespace llvm;

namespace logpool {

#include "./llvm_bc.h"

Module *LoadModule(LLVMContext &Context)
{
#if 1
    std::string ErrMsg;
    Module *M;
    MemoryBuffer *Buffer = MemoryBuffer::getMemBuffer(
            StringRef((const char *)bitcodes, sizeof(bitcodes)),
            "llvm_bitcode", false);
    assert(Buffer != 0);
    M = ParseBitcodeFile(Buffer, Context, &ErrMsg);
    assert(M != 0);
    return M;
#else
    return new Module("logpool_context", getGlobalContext());
#endif
}
}
