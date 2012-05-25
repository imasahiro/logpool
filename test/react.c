#include "reactive.h"
#include "lio/lio.h"
#include "lio/protocol.h"
#include "lio/util.h"

#include <stdio.h>
/* test */
#define TEST_ENTRY   100
#define TEST_WATCHER 10000
#define TEST_LOG     100000000
#define TEST_WATCHER_PER_ENTRY 100
#define TEST_TIME    100

static void watcher_watch(uintptr_t data, struct LogEntry *e)
{
    intptr_t *sum = (intptr_t *) data;
    *sum += 1;
}

static void watcher_remove(uintptr_t data)
{
    intptr_t *sum = (intptr_t *) data;
    *sum -= 1;
}

static void emit_log(react_engine_t *re, int i)
{
    char buf[1024];
    char data[128];
    snprintf(data, 128, "%d", i);
    int logSize = emit_message(buf, LOGPOOL_EVENT_WRITE, 3,
            strlen("TraceID"), strlen(data), "TraceID", data,
            strlen("key0"), strlen("val0"), "key0", "val0",
            strlen("key1"), strlen("val1"), "key1", "val1",
            strlen("key2"), strlen("val2"), "key2", "val2");
    react_engine_append_log(re, (struct Log *) buf, logSize);
}

int main(int argc, char const* argv[])
{
    react_engine_t *re = react_engine_new(4);
    reaction_entry_t e = {};
    intptr_t sum = 0;
    react_watcher_t  w = {(intptr_t)&sum, TEST_TIME,
        watcher_watch, watcher_remove};
    int i;
    for (i = 0; i < TEST_ENTRY; ++i) {
        char data[128] = {};
        snprintf(data, 128, "%d", i % TEST_WATCHER_PER_ENTRY);
        react_engine_append(re, data, strlen(data), &e);
    }
    fprintf(stderr, "mapsize=%d\n", poolmap_size(re->react_entries));
    for (i = 0; i < TEST_WATCHER; ++i) {
        //if (i % 100 == 0) fprintf(stderr, "W:%d\n", i);
        char data[128] = {};
        snprintf(data, 128, "%d", i % TEST_WATCHER_PER_ENTRY);
        react_engine_append_watcher(re, data, strlen(data), &w);
    }
    for (i = 0; i < TEST_LOG; ++i) {
        //if (i % 100 == 0) fprintf(stderr, "L:%d\n", i);
        emit_log(re, i % TEST_WATCHER_PER_ENTRY);
    }

    react_engine_delete(re);
    fprintf(stderr, "sum=%ld\n", sum);
    //assert(sum == 0);
    fprintf(stderr, "%ld\n", sizeof(struct reaction_entry));
    return 0;
}
