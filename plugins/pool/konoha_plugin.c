#include <libmemcached/memcached.h>
#include <konoha2/konoha2.h>
#include <konoha2/sugar.h>
#include "pool_plugin.h"

#ifndef K_PREFIX
#define K_PREFIX  "/usr/local"
#endif

static const char* packname(const char *str)
{
	char *p = strrchr(str, '.');
	return (p == NULL) ? str : (const char*)p+1;
}

static const char* packagepath(char *buf, size_t bufsiz, const char *fname)
{
	char *path = getenv("KONOHA_PACKAGEPATH"), *local = "";
	if(path == NULL) {
		path = getenv("KONOHA_HOME");
		local = "/package";
	}
	if(path == NULL) {
		path = getenv("HOME");
		local = "/.konoha2/package";
	}
	snprintf(buf, bufsiz, "%s%s/%s/%s_glue.k", path, local, fname, packname(fname));
#ifdef K_PREFIX
	FILE *fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
	}
	else {
		snprintf(buf, bufsiz, K_PREFIX "/konoha2/package" "/%s/%s_glue.k", fname, packname(fname));
	}
#endif
	return (const char*)buf;
}

static const char* exportpath(char *pathbuf, size_t bufsiz, const char *pname)
{
	char *p = strrchr(pathbuf, '/');
	snprintf(p, bufsiz - (p  - pathbuf), "/%s_exports.k", packname(pname));
	FILE *fp = fopen(pathbuf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char*)pathbuf;
	}
	return NULL;
}

const kplatform_t* platform_shell(void)
{
	static kplatform_t plat = {
		.name          = "shell",
		.stacksize     = K_PAGESIZE * 4,
		.malloc        = malloc,
		.free          = free,
		.realpath      = realpath,
		.fopen         = fopen,
		.fgetc         = fgetc,
		.feof          = feof,
		.fclose        = fclose,
		.packagepath   = packagepath,
		.exportpath    = exportpath,
	};
	return (const kplatform_t*)(&plat);
}

struct kRawPtr {
	kObjectHeader h;
	void *rawptr;
};

extern kstatus_t MODSUGAR_eval(CTX, const char *script, kline_t uline);

void konoha_plugin_init(konoha_t *konohap, memcached_st **mcp)
{
    *konohap = konoha_open(platform_shell());
    *mcp = memcached_create(NULL);
    CTX_t _ctx = *konohap;
    kKonohaSpace *ks = kmodsugar->rootks;
    KEXPORT_PACKAGE("sugar", ks, 0);
    KEXPORT_PACKAGE("logpool", ks, 0);
    memcached_server_list_st servers;
    memcached_return_t rc;
    servers = memcached_server_list_append(NULL, "127.0.0.1", 11211, &rc);
    if (rc != MEMCACHED_SUCCESS) {
        fprintf(stderr, "memcached_server_list_append failed\n");
    }
    rc = memcached_server_push(*mcp, servers);
    memcached_server_list_free(servers);
}

struct pool_plugin *konoha_plugin_get(konoha_t konoha, memcached_st *mc, char *buf, size_t len)
{
    size_t vlen;
    uint32_t flags;
    memcached_return_t rc;
    char *script = memcached_get(mc, buf, strlen(buf), &vlen, &flags, &rc);
    CTX_t _ctx = konoha;
    MODSUGAR_eval(_ctx, script, 0);
    kKonohaSpace *ks = kmodsugar->rootks;
    kMethod *mtd = kKonohaSpace_getMethodNULL(ks, TY_System, MN_("initPlugin"));
    if (mtd) {
        BEGIN_LOCAL(lsfp, K_CALLDELTA + 2);
        KSETv(lsfp[K_CALLDELTA+0].o, K_NULL);
        KCALL(lsfp, 0, mtd, 0, K_NULL);
        END_LOCAL();
        kObject *ret = lsfp[0].o;
        struct pool_plugin *plugin = (struct pool_plugin *) ((struct kRawPtr*) ret)->rawptr;
        plugin = pool_plugin_init(plugin);
        return plugin;
    }
    return NULL;
}

