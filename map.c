#define MEMORY_PREFIX map
#include "memory.h"
#undef MEMORY_PREFIX

#include "hash.h"
#include "map.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POOLMAP_INITSIZE      16

static void pmap_record_copy(pmap_record_t *dst, const pmap_record_t *src)
{
#if 0
    dst->hash = src->hash;
    dst->k    = src->k;
    dst->v    = src->v;
#else
    memcpy(dst, src, sizeof(pmap_record_t));
#endif
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
    m->records = cast(pmap_record_t *, map_do_malloc(alloc_size));
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
            ++m->used_size;
            return;
        }
        if (r->hash == rec->hash && m->fcmp(r->k, rec->k)) {
            m->ffree(r);
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
        }
    }
    map_do_free(old, oldsize*sizeof(pmap_record_t));
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

poolmap_t* poolmap_new(uint32_t init, fn_keygen fkey, fn_keycmp fcmp, fn_efree ffree)
{
    poolmap_t *m = cast(poolmap_t *, map_do_malloc(sizeof(*m)));
    if (init < POOLMAP_INITSIZE)
        init = POOLMAP_INITSIZE;
    pmap_record_reset(m, 1U << (SizeToKlass(init)));
    m->fkey  = fkey;
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

    map_do_free(m->records, m->record_size * sizeof(pmap_record_t));
    map_do_free(m, sizeof(*m));
}

pmap_record_t *poolmap_get(poolmap_t *m, char *key, uint32_t klen)
{
    uint32_t hash = djbhash(key, klen);
    pmap_record_t *r = pmap_get_(m, hash, m->fkey(key, klen));
    return r;
}

void poolmap_set(poolmap_t *m, char *key, uint32_t klen, void *val)
{
    pmap_record_t r;
    r.hash = djbhash(key, klen);
    r.k  = m->fkey(key, klen);
    r.v  = cast(uintptr_t, val);
    r.v2 = 0;
    pmap_set_(m, &r);
}

void poolmap_set2(poolmap_t *m, char *key, uint32_t klen, void *v1, uint32_t v2)
{
    pmap_record_t r;
    r.hash = djbhash(key, klen);
    r.k  = m->fkey(key, klen);
    r.v  = cast(uintptr_t, v1);
    r.v2 = v2;
    pmap_set_(m, &r);
}

void poolmap_remove(poolmap_t *m, char *key, uint32_t klen)
{
    uint32_t hash = djbhash(key, klen);
    pmap_record_t *r = pmap_get_(m, hash, m->fkey(key, klen));
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
    return 0;
}

int pool_global_deinit(void)
{
    CHECK_MALLOCED_SIZE(map);
    return 0;
}

#ifdef __cplusplus
}
#endif
