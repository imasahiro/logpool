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

#ifdef __cplusplus
extern "C" {
#endif

typedef void llcache_t;
llcache_t *llcache_new(const char *host, long port);
void  llcache_set(llcache_t * llmc, const char *key, const char *filename);
void *llcache_get(llcache_t * llmc, const char *key);
void  llcache_delete(llcache_t * llmc);

#ifdef __cplusplus
}
#endif


#endif /* end of include guard */
