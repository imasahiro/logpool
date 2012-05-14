#include "lio.h"
#include "stream.h"
#include "util.h"
#include <assert.h>
#include <event2/event.h>
#include <event2/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lio *lio_open(char *host, int port, int mode, struct lio_api *api)
{
    struct lio *lio;
    short ev_mode = 0;
    static int once = 1;
    if (once) {
        once = 0;
        evthread_use_pthreads();
    }

    if (mode & LIO_MODE_READ)
        ev_mode |= EV_READ;
    if (mode & LIO_MODE_WRITE)
        ev_mode |= EV_WRITE;

    lio = malloc(sizeof(*lio));
    lio->flags = mode & 0x3;
    lio->f_read  = api->f_read;
    lio->f_write = api->f_write;
    lio->f_close = api->f_close;
    lio->buffer  = malloc(LIO_BUFFER_SIZE);
    lio->shift   = 0;
    lio->last_buf = NULL;
    api->f_init(lio, host, port, ev_mode);
    return lio;
}

int lio_close(struct lio *lio)
{
    lio->f_close(lio);
    free(lio->buffer);
    bzero(lio, sizeof(*lio));
    free(lio);
    return LIO_OK;
}

int lio_write(struct lio *lio, const void *data, uint32_t nbyte)
{
    return lio->f_write(lio, data, nbyte);
}

int lio_read(struct lio *lio, void *data, uint32_t nbyte)
{
    return lio->f_read(lio, data, nbyte);
}

int lio_sync(struct lio *lio)
{
    (void)lio;
    return LIO_OK;
}

int lio_dispatch(struct lio *lio)
{
    assert(lio->base);
    fprintf(stderr, "dispach start\n");
    event_base_dispatch(lio->base);
    lio_close(lio);
    return 0;
}

#ifdef __cplusplus
}
#endif
