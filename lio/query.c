#include "lio.h"
#include "util.h"
#include "query.h"
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

static void query_dump(char *prefix, int idx, struct query_entry *e)
{
    debug_print(9, "%s %d e = {c=%p, q=('%s', %d)",
            prefix, idx, e->c, e->query, e->query_length);
}

void query_exec(struct Log *log, int log_size, struct query_list *q)
{
    int i;
    struct query_entry *e;
    for (i = 0; i < q->size; ++i) {
        e = q->list+i;
        assert(e->code);
        if (qengine_exec(e->code, log) != 0) {
            query_dump("E", i, e);
            bufferevent_write(e->c, log, log_size);
        }
    }
}

struct query_list *query_new(void)
{
    struct query_list *l;
    l = malloc(sizeof(*l));
    l->list = malloc(4 * sizeof(struct query_entry));
    l->capacity = 4;
    l->size = 0;
    return l;
}

void query_delete(struct query_list *l)
{
    free(l->list);
    bzero(l, sizeof(struct query_list));
    free(l);
}

void swap(struct query_entry *e1, struct query_entry *e2)
{
    struct query_entry e0;
    e0  = *e1;
    *e1 = *e2;
    *e2 = e0;
}

void query_delete_connection(struct query_list *q, struct bufferevent *bev)
{
    int i;
    struct query_entry *e;
    for (i = 0; i < q->size; ++i) {
        e = q->list+i;
        if (e->c == bev) {
            query_dump("D", i, e);
            // TODO free(e.query); e.query = NULL;
            swap(e, q->list+q->size-1);
            q->size -= 1;
            bzero(q->list+q->size, sizeof(struct query_entry));
        }
    }
}

static void query_add0(struct query_list *q, struct query_entry *e)
{
    if (q->size + 1 >= q->capacity) {
        size_t esize = sizeof(struct query_entry);
        q->capacity *= 2;
        q->list = realloc(q->list, esize * q->capacity);
    }
    memcpy(q->list+q->size, e, sizeof(struct query_entry));
    query_dump("A", q->size, e);
    q->size++;
}

void query_add(struct qengine *engine, struct Query *q, struct bufferevent *bev, struct query_list *l)
{
    struct query_entry e;
    e.query_length = q->vlen;
    e.query = malloc(q->vlen);
    memcpy((char*)e.query, q->data, e.query_length);
    e.c = bev;
    e.code = qengine_compile(engine, (char*)e.query);
    query_add0(l, &e);

    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    bufferevent_set_timeouts(bev, NULL, &tv);
}

#ifdef __cplusplus
}
#endif
