#include "pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void poolmap_dump(poolmap_t *m)
{
    poolmap_iterator itr = {0};
    pmap_record_t *r;
    int i = 0;
    while ((r = poolmap_next(m, &itr)) != NULL) {
        ++i;
#if 0
        fprintf(stderr, "{h=0x%08x, key=%s, v=%s}\n",
                r->hash, (char*)r->k, (char*)r->v);
#endif
    }
    fprintf(stderr, "poolmap.size:%d\n", poolmap_size(m));
    assert(poolmap_size(m) == 38201);
}

static void load(poolmap_t *m, const char *fname)
{
    char buffer[32];
    FILE *fp = fopen(fname, "r");
    while(fgets(buffer , sizeof(buffer), fp) != NULL) {
        int len = strlen(buffer);
        char *s = malloc(len);
        bzero(s, len);
        memcpy(s, buffer, len-1);
        poolmap_set(m, s, len, s, len);
    }
    fclose(fp);
}

static int entry_key_eq(uintptr_t k0, uintptr_t k1)
{
    char *s0 = (char *) k0;
    char *s1 = (char *) k1;
    return strcmp(s0, s1) == 0;
}

static void entry_free(pmap_record_t *r)
{
    char *str = (char *) r->k;
    free(str);
}

int main(int argc, char const* argv[])
{
    int i;
    pool_global_init();
    poolmap_t *map = poolmap_new(4, entry_key_eq, entry_free);
    assert(argc == 2);
    for (i = 0; i < 100; ++i) {
        load(map, argv[1]);
    }
    poolmap_dump(map);
    poolmap_delete(map);
    pool_global_deinit();
    return 0;
}
