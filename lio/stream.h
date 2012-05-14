#include "protocol.h"
#include <event2/bufferevent.h>

#ifndef LIO_STREAM_H
#define LIO_STREAM_H

struct lio;
struct chunk_stream {
    struct lio *lio;
    struct bufferevent *bev;
    char *cur;
    int   len;
};

int chunk_stream_empty(struct chunk_stream *cs);
struct chunk_stream *chunk_stream_init(struct chunk_stream *cs, struct lio *lio, struct bufferevent *bev);
void chunk_stream_deinit(struct chunk_stream *cs);
struct log_data *chunk_stream_get(struct chunk_stream *cs, int *log_size);

#endif /* end of include guard */
