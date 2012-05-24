#include <stdlib.h>
#include <string.h>

#ifndef LOGPOOL_MEMORY_H
#define LOGPOOL_MEMORY_H

#ifdef USE_NO_CHECK_MALLOC
#define CHECK_MALLOC(PREFIX)
#define CHECK_MALLOCED_SIZE(PREFIX)
#define CHECK_MALLOCED_INC_SIZE(PREFIX, n)
#define CHECK_MALLOCED_DEC_SIZE(PREFIX, n)
#else /*defined(USE_CHECK_MALLOC)*/
#define CONCAT(A, B) A##_##B
#define CHECK_MALLOC(PREFIX) static size_t CONCAT(PREFIX,malloced_size) = 0
#define CHECK_MALLOCED_SIZE(PREFIX)      assert(CONCAT(PREFIX,malloced_size) == 0)
#define CHECK_MALLOCED_INC_SIZE(PREFIX, n) (CONCAT(PREFIX,malloced_size) += (n))
#define CHECK_MALLOCED_DEC_SIZE(PREFIX, n) (CONCAT(PREFIX,malloced_size) -= (n))
CHECK_MALLOC(MEMORY_PREFIX);
#endif /*defined(USE_CHECK_MALLOC)*/

static inline void do_bzero(void *ptr, size_t size)
{
    memset(ptr, 0, size);
}

static inline void *do_malloc(size_t size)
{
    void *ptr = malloc(size);
    do_bzero(ptr, size);
    CHECK_MALLOCED_INC_SIZE(MEMORY_PREFIX, size);
    return ptr;
}

static inline void do_free(void *ptr, size_t size)
{
    do_bzero(ptr, size);
    CHECK_MALLOCED_DEC_SIZE(MEMORY_PREFIX, size);
    free(ptr);
}

static inline void check_memory()
{
}


#endif /* end of include guard */
