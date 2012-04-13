#include <libmemcached/memcached.h>
#include <llvm/Module.h>

#ifndef LLCACHE_H_
#define LLCACHE_H_

namespace logpool {
class llmc {
private:
    memcached_st *st;
public:
    llmc(const std::string host, long port) { init(host, port); }
    void init(const std::string host, long port);
    void set(const std::string key, llvm::Function *F);
    llvm::Function *get(const std::string key, llvm::Module *m);
};
}

#endif /* end of include guard */
