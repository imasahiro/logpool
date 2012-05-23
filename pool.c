#include "pool.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POOLMAP_INITSIZE      16
#define cast(T, V) ((T)(V))
#define CLZ(n) __builtin_clzl(n)
#define BITS (sizeof(void*) * 8)
#define SizeToKlass(N) ((uint32_t)(BITS - CLZ(N - 1)))

static size_t malloced_size = 0;
#define CHECK_MALLOCED_SIZE()      assert(malloced_size == 0)
#define CHECK_MALLOCED_INC_SIZE(n) (malloced_size += (n))
#define CHECK_MALLOCED_DEC_SIZE(n) (malloced_size -= (n))

static uint32_t djbhash(const char *p, uint32_t len)
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

static inline void do_bzero(void *ptr, size_t size)
{
    memset(ptr, 0, size);
}

static inline void *do_malloc(size_t size)
{
    void *ptr = malloc(size);
    do_bzero(ptr, size);
    CHECK_MALLOCED_INC_SIZE(size);
    return ptr;
}

static inline void do_free(void *ptr, size_t size)
{
    do_bzero(ptr, size);
    CHECK_MALLOCED_DEC_SIZE(size);
    free(ptr);
}

static void pmap_record_copy(pmap_record_t *dst, const pmap_record_t *src)
{
    dst->hash = src->hash;
    dst->k    = src->k;
    dst->v    = src->v;
}

static inline pmap_record_t *pmap_at(poolmap_t *m, uint32_t idx)
{
    assert(idx < m->record_size);
    return m->records+idx;
}

static void pmap_record_reset(poolmap_t *m, size_t newsize)
{
    uint32_t alloc_size = sizeof(pmap_record_t) * newsize;
    m->used_size = 0;
    m->record_size = newsize;
    m->records = cast(pmap_record_t *, do_malloc(alloc_size));
    m->mask = m->record_size - 1;
}

static void pmap_set_no_resize(poolmap_t *m, pmap_record_t *rec)
{
    uint32_t i = 0, idx = rec->hash & m->mask;
    pmap_record_t *r;
    do {
        r = m->records+idx;
        if (r->hash == 0) {
            pmap_record_copy(r, rec);
            m->used_size++;
            return;
        }
        if (r->hash == rec->hash && m->fcmp(r->k, rec->k)) {
            r->v = rec->v;
            return;
        }
        idx = (idx + 1) & m->mask;
    } while (i++ < m->used_size);
}

static void pmap_record_resize(poolmap_t *m)
{
    pmap_record_t *old = m->records;
    uint32_t i, oldsize = m->record_size;

    pmap_record_reset(m, oldsize*2);
    for (i = 0; i < oldsize; ++i) {
        pmap_record_t *r = old + i;
        if (r->hash) {
            pmap_set_no_resize(m, r);
            do_bzero(r, sizeof(*r));
        }
    }
    do_free(old, oldsize*sizeof(pmap_record_t));
}

static void pmap_set_(poolmap_t *m, pmap_record_t *rec)
{
    if (m->used_size > m->record_size * 3 / 4) {
        pmap_record_resize(m);
    }
    pmap_set_no_resize(m, rec);
}

static pmap_record_t *pmap_get_(poolmap_t *m, uint32_t hash, uintptr_t key)
{
    uint32_t i = 0;
    uint32_t idx = hash & m->mask;
    do {
        pmap_record_t *r = pmap_at(m, idx);
        if (r->hash == hash && m->fcmp(r->k, key)) {
            return r;
        }
        idx = (idx + 1) & m->mask;
    } while (i++ < m->used_size);
    return NULL;
}

poolmap_t* poolmap_new(uint32_t init, fn_keycmp fcmp, fn_efree ffree)
{
    poolmap_t *m = cast(poolmap_t *, do_malloc(sizeof(*m)));
    if (init < POOLMAP_INITSIZE)
        init = POOLMAP_INITSIZE;
    pmap_record_reset(m, 1U << (SizeToKlass(init)));
    m->fcmp  = fcmp;
    m->ffree = ffree;
    return m;
}

void poolmap_delete(poolmap_t *m)
{
    assert(m != 0);
    uint32_t i, size = m->record_size;
    for (i = 0; i < size; ++i) {
        pmap_record_t *r = pmap_at(m, i);
        if (r->hash) {
            m->ffree(r);
        }
    }

    do_free(m->records, m->record_size * sizeof(pmap_record_t));
    do_free(m, sizeof(*m));
}

pmap_record_t *poolmap_get(poolmap_t *m, char *key, uint32_t klen)
{
    uint32_t hash = djbhash(key, klen);
    pmap_record_t *r = pmap_get_(m, hash, (uintptr_t)key);
    return r;
}

void poolmap_set(poolmap_t *m, char *key, uint32_t klen, void *val, uint32_t vlen)
{
    pmap_record_t r;
    r.k = cast(uintptr_t, key);
    r.v = cast(uintptr_t, val);
    r.hash = djbhash(key, klen);
    pmap_set_(m, &r);
}

void poolmap_remove(poolmap_t *m, char *key, uint32_t klen)
{
    uint32_t hash = djbhash(key, klen);
    pmap_record_t *r = pmap_get_(m, hash, (uintptr_t)key);
    if (r) {
        r->hash = 0;
        r->k = 0;
        m->used_size -= 1;
    }
}

pmap_record_t *poolmap_next(poolmap_t *m, poolmap_iterator *itr)
{
    uint32_t i, size = m->record_size;
    for (i = itr->index; i < size; ++i) {
        pmap_record_t *r = pmap_at(m, i);
        if (r->hash) {
            itr->index = i+1;
            return r;
        }
    }
    itr->index = i;
    assert(itr->index == size);
    return NULL;
}

int pool_global_init(void)
{
    malloced_size = 0;
    return 0;
}
int pool_global_deinit(void)
{
    CHECK_MALLOCED_SIZE();
    return 0;
}

#ifdef __cplusplus
}
#endif
