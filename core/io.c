#include "io.h"
#include "stream.h"
#include <assert.h>
#include <event2/event.h>
#include <event2/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

struct io *io_open(char *host, int port, int mode, struct io_api *api)
{
    struct io *io;
    short ev_mode = 0;
    static int once = 1;
    if (once) {
        once = 0;
        evthread_use_pthreads();
    }

    if (mode & IO_MODE_READ)
        ev_mode |= EV_READ;
    if (mode & IO_MODE_WRITE)
        ev_mode |= EV_WRITE;

    io = malloc(sizeof(*io));
    io->flags = mode & 0x3;
    io->f_read  = api->f_read;
    io->f_write = api->f_write;
    io->f_close = api->f_close;
    api->f_init(io, host, port, ev_mode);
    return io;
}

int io_close(struct io *io)
{
    io->f_close(io);
    bzero(io, sizeof(*io));
    free(io);
    return IO_OK;
}

int io_write(struct io *io, const void *data, uint32_t nbyte)
{
    return io->f_write(io, data, nbyte);
}

int io_read(struct io *io, void *data, uint32_t nbyte)
{
    return io->f_read(io, data, nbyte);
}

int io_sync(struct io *io)
{
    (void)io;
    return IO_OK;
}

int io_dispatch(struct io *io)
{
    assert(io->base);
    fprintf(stderr, "dispach start\n");
    event_base_dispatch(io->base);
    io_close(io);
    return 0;
}

struct io *io_open_trace(char *host, int port)
{
    extern struct io_api trace_api;
    struct io *io = io_open(host, port,
            IO_MODE_READ|IO_MODE_WRITE, &trace_api);
    return io;
}

#ifdef __cplusplus
}
#endif
