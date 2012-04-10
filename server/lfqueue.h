#include <stdint.h>

#ifndef LFQUEUE_H_
#define LFQUEUE_H_

namespace logpool {

typedef uintptr_t Data;
struct EntryTag {
    int32_t ver;
    int32_t count;
};

struct ExitTag {
    int16_t count;
    int16_t outstandingTransfers;
    int8_t nlP;
    int8_t toBeFreed;
} __attribute__((aligned(8)));

struct Node {
    struct Node *pred;
    struct Node *next;
    Data d;
    ExitTag exit;
    Node(Data d);
};

struct Loc {
    Node *ptr0;
    Node *ptr1;
    EntryTag entry;
};

struct ThreadContext {
    int myver;
    Node *mynode;
};

struct Queue {
    Loc Tail;
    Loc Head;
    Queue(void);
    void enq(ThreadContext &ctx, Data d);
    Data deq(ThreadContext &ctx);
    void dump() const;
    bool isEmpty() const;
};

} /* namespace logpool */
#endif /* end of include guard */
