#include "qengine.h"
#include "lio.h"
#include "protocol.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum QENGINE_OPCODE {
    OPCODE_nop,
    OPCODE_append /* append key val */,
    OPCODE_match  /* match  dst src */,
    OPCODE_gt     /* gt log['key'] 30 */,
    OPCODE_ge     /* ge log['key'] 30 */,
    OPCODE_lt     /* lt log['key'] 30 */,
    OPCODE_le     /* lt log['key'] 30 */,
    OPCODE_eq     /* eq log['key'] 30 */,
    OPCODE_ne     /* ne log['key'] 30 */,
    QENGINE_OPMAX,
    OPCODE_ret = QENGINE_OPMAX,
};

struct qengine_op {
    const char *name;
    uint16_t oplen;
    uint16_t opcode;
};

static const struct qengine_op qengine_ops[] = {
#define OP(op) {#op, sizeof(#op) - 1, OPCODE_##op}
    OP(nop),
    OP(append),
    OP(match),
    OP(gt),
    OP(ge),
    OP(lt),
    OP(le),
    OP(eq),
    OP(ne),
    OP(ret),
#undef OP
};

struct qcode {
    enum QENGINE_OPCODE opcode:8;
    unsigned char key[8];
    unsigned char val[7];
};

void qcode_dump(struct qcode *c, FILE *fp)
{
    struct qcode *pc = c;
    while (pc->opcode != OPCODE_ret) {
        const char *name = qengine_ops[c->opcode].name;
        fprintf(fp, "[%02d] %s '%s' '%s'\n",
                (int)(pc - c), name, pc->key, pc->val);
        ++pc;
    }
}

struct qcode_list *qcode_list_init(struct qcode_list *qlist)
{
    qlist->c = malloc(sizeof(struct qcode) * 2);
    qlist->size = 0;
    qlist->capacity = 2;
    return qlist;
}

void qcode_list_append(struct qcode_list *qlist, struct qcode *c)
{
    if (qlist->size + 1 >= qlist->capacity) {
        size_t esize = sizeof(struct qcode);
        qlist->capacity *= 2;
        qlist->c = realloc(qlist->c, esize * qlist->capacity);
    }
    memcpy(qlist->c+qlist->size, c, sizeof(struct qcode));
    qlist->size++;
}

void qcode_list_deinit(struct qcode_list *qlist)
{
    qlist->size = 0;
}

static size_t compile_nop(struct qcode_list *qlist, char *q, size_t len)
{
    assert(0 && "TODO");
    return len;
}

static size_t compile_helper(struct qcode_list *qlist, char *q, size_t len, enum QENGINE_OPCODE opcode)
{
    char *qbase = q;
    struct qcode c = {0};
    size_t oplen = qengine_ops[opcode].oplen;
    c.opcode = qengine_ops[opcode].opcode;
    assert(strncmp(q, qengine_ops[opcode].name, oplen) == 0);
    q += oplen + 1;
    char *v = strchr(q, ' ')+1;
    char *e = strchr(v, ' ');
    if (!e)
        e = qbase + len;
    memcpy(c.key, q, v - q - 1);
    memcpy(c.val, v, e - v);
    qcode_list_append(qlist, &c);
    return e - qbase;
}

static size_t compile_match(struct qcode_list *qlist, char *q, size_t len)
{
    return compile_helper(qlist, q, len, OPCODE_match);
}

static size_t compile_gt(struct qcode_list *qlist, char *q, size_t len)
{
    return compile_helper(qlist, q, len, OPCODE_gt);
}

typedef size_t (*fqcode_compile)(struct qcode_list *qlist, char *q, size_t len);
static fqcode_compile qcode_compile[] = {
    compile_nop,
    compile_nop, // append
    compile_match,
    compile_gt,
    compile_nop, //ge
    compile_nop, //lt
    compile_nop, //le
    compile_nop, //eq
    compile_nop, //ne
};

struct qengine {
    struct qcode **compiled_code;
    size_t size;
    size_t capacity;
};

int qengine_exec(struct qcode *code, struct Log *log)
{
    struct qcode *pc = code;
    char *data = log_get_data(log);
    int state = 0;
#define NEXT() ++pc; goto L_head
    L_head:;
    switch (pc->opcode) {
    case OPCODE_nop:
        NEXT();
    case OPCODE_append:
        assert(0 && "TODO");
        NEXT();
    case OPCODE_match: {
        size_t i;
        for (i = 0; i < log->logsize; ++i) {
            uint16_t klen, vlen;
            klen = log_get_length(log, i*2+0);
            vlen = log_get_length(log, i*2+1);
            char *key = data;
            char *val = data+klen;
            data += klen + vlen;
            if (strncmp(key, (char *) pc->key, klen) != 0) {
                continue;
            }
            if (strncmp(val, (char *) pc->val, vlen) == 0) {
                /* match */
                state = 1;
                continue;
            }
        }
        NEXT();
    }
    case OPCODE_gt:
        NEXT();
    case OPCODE_ge:
    case OPCODE_lt:
    case OPCODE_le:
    case OPCODE_eq:
    case OPCODE_ne:
        assert(0 && "TODO");
        NEXT();
    case OPCODE_ret:
        return state;
    default:
        assert(0 && "TODO");
        NEXT();
    }
    return 0;
}

static int skip_space(char *q, size_t i, size_t len)
{
    for (; i < len; ++i) {
        if (q[i] != ' ')
            return i;
    }
    return i;
}

struct qcode *qengine_compile(struct qengine *e, char *q)
{
    size_t i, len = strlen(q);
    struct qcode_list qlist_, *qlist = qcode_list_init(&qlist_);
    for (i = skip_space(q, 0, len); i < len;
            i = skip_space(q, i+1, len)) {
        size_t j;
        for (j = 0; j < QENGINE_OPMAX; ++j) {
            if (strncmp(q+i, qengine_ops[j].name, qengine_ops[j].oplen) == 0) {
                i = qcode_compile[j](qlist, q+i, len-i);
            }
        }
    }
    struct qcode ret = {OPCODE_ret, {}, {}};
    qcode_list_append(qlist, &ret);

    struct qcode *c = qlist->c;
    if (e->size + 1 >= e->capacity) {
        size_t esize = sizeof(struct qcode*);
        e->capacity *= 2;
        e->compiled_code = realloc(e->compiled_code, esize * e->capacity);
    }
    e->compiled_code[e->size] = c;
    e->size++;
    return c;
}

struct qengine *qengine_init(void)
{
    struct qengine *e = malloc(sizeof(*e));
    bzero(e, sizeof(*e));
    e->compiled_code = malloc(4 * sizeof(struct qcode *));
    e->capacity = 4;
    e->size = 0;
    return e;
}

void qengine_exit(struct qengine *e)
{
    size_t i;
    assert(e);
    for (i = 0; i < e->size; ++i) {
        free(e->compiled_code[i]);
        e->compiled_code[i] = NULL;
    }
    free(e->compiled_code);
    bzero(e, sizeof(*e));
    free(e);
}

#ifdef __cplusplus
}
#endif
