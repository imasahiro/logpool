#include "message.idl.data.h"
#include "qengine.h"

#ifndef LIO_QUERY_H
#define LIO_QUERY_H

#ifdef __cplusplus
extern "C" {
#endif

struct query_entry {
    struct bufferevent *c;
    uint32_t    flags;
    uint32_t    query_length;
    const char *query;
    struct qcode *code;
};

struct query_list {
    struct query_entry *list;
    int size;
    int capacity;
};

void query_exec(struct Log *log, int log_size, struct query_list *q);
struct query_list *query_new(void);
void query_delete(struct query_list *l);
void query_add(struct qengine *e, struct Query *q, struct bufferevent *bev, struct query_list *l);
void query_delete_connection(struct query_list *q, struct bufferevent *bev);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard */
