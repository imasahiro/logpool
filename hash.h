#include <stdint.h>
#ifndef LOGPOOL_HASH_
#define LOGPOOL_HASH_

static inline uint32_t djbhash(const char *p, uint32_t len)
{
    uint32_t hash = 5381;
    uint32_t n = (len + 3) / 4;
    /* Duff's device */
    switch(len%4){
    case 0: do{ hash = ((hash << 5) + hash) + *p++;
    case 3:     hash = ((hash << 5) + hash) + *p++;
    case 2:     hash = ((hash << 5) + hash) + *p++;
    case 1:     hash = ((hash << 5) + hash) + *p++;
            } while(--n>0);
    }
    return (hash & 0x7fffffff);
}

#endif /* end of include guard */
