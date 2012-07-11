#include <libmemcached/memcached.h>
#include <konoha2/konoha2.h>
#include <konoha2/sugar.h>
#include "pool_plugin.h"
#include <stdio.h>
#include <syslog.h>

#ifndef K_PREFIX
#define K_PREFIX  "/usr/local"
#endif

static const char* l_packname(const char *str)
{
	char *p = strrchr(str, '.');
	return (p == NULL) ? str : (const char*)p+1;
}

static const char* l_packagepath(char *buf, size_t bufsiz, const char *fname)
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
	snprintf(buf, bufsiz, "%s%s/%s/%s_glue.k", path, local, fname, l_packname(fname));
#ifdef K_PREFIX
	FILE *fp = fopen(buf, "r");
	if(fp != NULL) {
		fclose(fp);
	}
	else {
		snprintf(buf, bufsiz, K_PREFIX "/konoha2/package" "/%s/%s_glue.k", fname, l_packname(fname));
	}
#endif
	return (const char*)buf;
}

static const char* l_exportpath(char *pathbuf, size_t bufsiz, const char *pname)
{
	char *p = strrchr(pathbuf, '/');
	snprintf(p, bufsiz - (p  - pathbuf), "/%s_exports.k", l_packname(pname));
	FILE *fp = fopen(pathbuf, "r");
	if(fp != NULL) {
		fclose(fp);
		return (const char*)pathbuf;
	}
	return NULL;
}

static const char* begin(kinfotag_t t) { (void)t; return ""; }
static const char* end(kinfotag_t t) { (void)t; return ""; }

static void dbg_p(const char *file, const char *func, int L, const char *fmt, ...)
{
	va_list ap;
	va_start(ap , fmt);
	fflush(stdout);
	fprintf(stderr, "DEBUG(%s:%s:%d) ", file, func, L);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}


static const kplatform_t logpool_platform = {
	.name        = "logpool",
	.stacksize   = K_PAGESIZE,
	.malloc_i    = malloc,
	.free_i      = free,
	.setjmp_i    = ksetjmp,
	.longjmp_i   = klongjmp,
	.realpath_i  = realpath,
	.fopen_i     = (FILE_i* (*)(const char*, const char*))fopen,
	.fgetc_i     = (int     (*)(FILE_i *))fgetc,
	.feof_i      = (int     (*)(FILE_i *))feof,
	.fclose_i    = (int     (*)(FILE_i *))fclose,
	.syslog_i    = syslog,
	.vsyslog_i   = vsyslog,
	.printf_i    = printf,
	.vprintf_i   = vprintf,
	.snprintf_i  = snprintf,
	.vsnprintf_i = vsnprintf,
	.qsort_i     = qsort,
	.exit_i      = exit,
	.packagepath = l_packagepath,
	.exportpath  = l_exportpath,
	.begin       = begin,
	.end         = end,
	.dbg_p       = dbg_p,
};


struct kRawPtr {
	kObjectHeader h;
	void *rawptr;
};

extern kstatus_t MODSUGAR_eval(CTX, const char *script, kline_t uline);

void konoha_plugin_init(konoha_t *konohap, memcached_st **mcp)
{
    *konohap = konoha_open(&logpool_platform);
    *mcp = memcached_create(NULL);
    CTX_t _ctx = *konohap;
    kNameSpace *ks = KNULL(NameSpace);
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

struct pool_plugin *konoha_plugin_get(konoha_t konoha, memcached_st *mc, char *buf, size_t len, void *req)
{
    size_t vlen;
    uint32_t flags;
    memcached_return_t rc;
    char *script = memcached_get(mc, buf, strlen(buf), &vlen, &flags, &rc);
    CTX_t _ctx = konoha;
    kObject *ev = new_kObject(CT_Int/*Dummy*/, (uintptr_t)req);
    MODSUGAR_eval(_ctx, script, 0);
    kNameSpace *ks = KNULL(NameSpace);
    kMethod *mtd = kNameSpace_getMethodNULL(ks, TY_System, MN_("initPlugin"));
    if (mtd) {
        BEGIN_LOCAL(lsfp, K_CALLDELTA + 2);
        KSETv(lsfp[K_CALLDELTA+0].o, K_NULL);
        KSETv(lsfp[K_CALLDELTA+1].o, ev);
        KCALL(lsfp, 0, mtd, 2, K_NULL);
        END_LOCAL();
        kObject *ret = lsfp[0].o;
        struct pool_plugin *plugin = (struct pool_plugin *) ((struct kRawPtr*) ret)->rawptr;
        plugin = pool_plugin_init(plugin);
        return plugin;
    }
    return NULL;
}

