#include <vector>
#include <string>
#include <iostream>
#include <llvm/LLVMContext.h>
#include <llvm/Linker.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Bitcode/BitstreamWriter.h>
#include "llvm/Support/MemoryBuffer.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include "llcache.h"

using namespace std;
using namespace llvm;
using namespace logpool;

void llmc::init(const std::string host, long port)
{
    memcached_return_t rc;
    memcached_server_list_st servers;

    st = memcached_create(NULL);
    if (st == NULL) {
        /* TODO Error */
        abort();
    }
    servers = memcached_server_list_append(NULL, host.c_str(), port, &rc);
    if (rc != MEMCACHED_SUCCESS) {
        /* TODO Error */
        cerr <<"Error!! " << memcached_strerror(st, rc) << endl;
        abort();
    }
    rc = memcached_server_push(st, servers);
    if (rc != MEMCACHED_SUCCESS) {
        /* TODO Error */
        cerr <<"Error!! " << memcached_strerror(st, rc) << endl;
        abort();
    }
    memcached_server_list_free(servers);
}

static Function *Cloning(Module *M, Function *F)
{
    Function *NewF = CloneFunction(F);
    M->getFunctionList().push_back(NewF);
    return NewF;
}

void llmc::set(const std::string key, Function *F)
{
    LLVMContext &Context = getGlobalContext();
    Module *m = new Module("_", Context);
    Cloning(m, F);
    std::vector<unsigned char> Buffer;
    BitstreamWriter Stream(Buffer);
    WriteBitcodeToStream(m, Stream);

    std::cerr << "llmc::set do; '" << key << "'" << std::endl;
    memcached_return_t rc = memcached_set(st, key.c_str(), key.size(),
            (char *) &Buffer.front(), Buffer.size(), 0, 0);
    if (rc != MEMCACHED_SUCCESS) {
        std::cerr << "llmc::set failed; '" << key << "'" << std::endl;
    }
}

Function *llmc::get(const std::string key, Module *m)
{
    memcached_return_t rc;
    LLVMContext &Context = getGlobalContext();
    size_t vlen;
    uint32_t flags;
    char *value = memcached_get(st, key.c_str(), key.size(), &vlen, &flags, &rc);
    if (rc != MEMCACHED_SUCCESS) {
        std::cerr << "llmc::get failed; '" << key << "'" << std::endl;
        return NULL;
    }
    StringRef Input(value, vlen);
    MemoryBuffer *Buffer;
    Buffer = MemoryBuffer::getMemBuffer(Input, "<llmc>", false);
    std::string ErrMsg;
    Module *newm = ParseBitcodeFile(Buffer, Context, &ErrMsg);
    if (!newm) {
        std::cout << "Error" << ErrMsg << std::endl;
        return NULL;
    }
    if (Linker::LinkModules(m, newm, Linker::DestroySource, &ErrMsg)) {
        std::cout << "error" << ErrMsg << std::endl;
        return NULL;
    }
    return m->getFunction(key);
}
