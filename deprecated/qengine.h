#include "message.idl.data.h"
#include <stdio.h>

#ifndef QENGINE_H
#define QENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

struct qcode;
struct qengine;
int qengine_exec(struct qcode *code, struct Log *log);
struct qcode *qengine_compile(struct qengine *e, char *query);
struct qengine *qengine_init();
void qengine_exit(struct qengine *q);

struct qcode_list {
    struct qcode *c;
    size_t size;
    size_t capacity;
};

void qcode_dump(struct qcode *c, FILE *fp);
struct qcode_list *qcode_list_init(struct qcode_list *qlist);
void qcode_list_deinit(struct qcode_list *qlist);
void qcode_list_append(struct qcode_list *qlist, struct qcode *c);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard */
