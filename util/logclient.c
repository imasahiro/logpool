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
    logpool_query(logpool, "match tid tid1");
    struct Log *logbuf = alloca(sizeof(struct Log) + 256);
    while (1) {
        void *data_ = logpool_client_get(logpool, logbuf, 256);
        dump_log(stderr, "log=(", logbuf, ")\n");
        usleep(10);
        (void)data_;
    }
    logpool_close(logpool);
    return 0;
}
