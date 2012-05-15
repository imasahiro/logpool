#include "logpool.h"
#include "logpool_util.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "lio/lio.h"
#include "lio/protocol.h"
#include "lio/message.idl.data.h"
//static void read_log(struct lio *lio, struct Log *tmp)
//{
//    lio_read(lio, (char*) tmp, 128);
//}

int main(int argc, char **argv)
{
    logpool_t *logpool;
    logpool = logpool_open_client(NULL, "127.0.0.1", 14801);
    logpool_query(logpool, "match tid tid0");
    struct Log *logbuf = alloca(sizeof(struct Log) + 256);
    while (1) {
        char kbuf[64];
        char vbuf[64];
        void *data_ = logpool_client_get(logpool, logbuf, 256);
        char *data = log_get_data(logbuf);
        memcpy(kbuf, data,logbuf->klen);
        memcpy(vbuf, data+logbuf->klen, logbuf->vlen);
        fprintf(stderr, "log=(%d, %d, '%s': '%s')\n",
                logbuf->klen, logbuf->vlen, kbuf, vbuf);
        usleep(10);
        (void)data_;
    }
    logpool_close(logpool);
    return 0;
}
