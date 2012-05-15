#include <stdio.h>
#include "lfqueue.h"

namespace logpool {
#define CURRENT(loc, ver)        ((ver % 2 == 0)?( (loc)->ptr0):( (loc)->ptr1))
#define NONCURRENT(loc, ver)     ((ver % 2 != 0)?( (loc)->ptr0):( (loc)->ptr1))
#define NONCURRENTADDR(loc, ver) ((ver % 2 != 0)?(&(loc)->ptr0):(&(loc)->ptr1))
#define EXITTAG_INIT(e) do {\
    e.count = 0;\
    e.outstandingTransfers = 2;\
    e.nlP = 0;\
    e.toBeFreed = 0;\
} while (0)

#define CLEAN(exit) (exit.count == 0 && exit.outstandingTransfers == 0)
#define FREEABLE(exit) (CLEAN(exit) && exit.nlP && exit.toBeFreed)

#define ToU64(v) (*((uint64_t*)&v))

static inline bool CAS(uint64_t *ptr, uint64_t oldv, uint64_t newv)
{
    uint64_t result = __sync_val_compare_and_swap(ptr, oldv, newv);
    return ((result == oldv) ? true : false);
}

static Node *LL(Loc *loc, int *myver, Node **mynode)
{
    EntryTag e, newe;
    do {
        e = loc->entry;
        *myver  = e.ver;
        *mynode = CURRENT(loc, e.ver);
        newe.ver   = e.ver;
        newe.count = e.count + 1;
    } while (!CAS((uint64_t*)&loc->entry, ToU64(e), ToU64(newe)));
    return (*mynode);
}

static void Transfer(Node *node, int count)
{
    ExitTag exit, post;
    do {
        exit = node->exit;
        post.count = exit.count + count;
        post.outstandingTransfers = exit.outstandingTransfers - 1;
        post.nlP = exit.nlP;
        post.toBeFreed = exit.toBeFreed;
    } while (!CAS((uint64_t*)&node->exit, ToU64(exit), ToU64(post)));
}

static void SetNLPred(Node *node)
{
    ExitTag exit, post;
    do {
        exit = node->exit;
        post.count = exit.count;
        post.outstandingTransfers = exit.outstandingTransfers;
        post.nlP = true;
        post.toBeFreed = exit.toBeFreed;
    } while (!CAS((uint64_t*)&node->exit, ToU64(exit), ToU64(post)));
    if (FREEABLE(post)) {
        delete node;
    }
}

static void SetToBeFreed(Node *node)
{
    ExitTag exit, post;
    do {
        exit = node->exit;
        post.count = exit.count;
        post.outstandingTransfers = exit.outstandingTransfers;
        post.nlP = exit.nlP;
        post.toBeFreed = true;
    } while (!CAS((uint64_t*)&node->exit, ToU64(exit), ToU64(post)));
    if (FREEABLE(post)) {
        delete node;
    }
}

static void Release(Node *node)
{
    Node *pred = node->pred;
    ExitTag exit, post;
    do {
        exit = node->exit;
        post.count = exit.count - 1;
        post.outstandingTransfers = exit.outstandingTransfers;
        post.nlP = exit.nlP;
        post.toBeFreed = exit.toBeFreed;
    } while (!CAS((uint64_t*)&node->exit, ToU64(exit), ToU64(post)));
    if (CLEAN(post)) {
        SetNLPred(pred);
    }
    if (FREEABLE(post)) {
        delete node;
    }
}

static void Unlink(Loc *loc, int myver, Node *mynode)
{
    Release(mynode);
}

static bool SC(Loc *loc, Node *node, int myver, Node *mynode)
{
    EntryTag e, newe;
    Node *pred = mynode->pred;
    bool success = CAS((uint64_t*)NONCURRENTADDR(loc, myver), (uint64_t)pred, (uint64_t)node);

    e = loc->entry;
    while (e.ver == myver) {
        newe.ver   = e.ver + 1;
        newe.count = 0;
        if (CAS((uint64_t*)&loc->entry, ToU64(e), ToU64(newe))) {
            Transfer(mynode, e.count);
        }
        e = loc->entry;
    }
    Release(mynode);
    return success;
}

Node::Node(Data d) : next(NULL), d(d) {
    EXITTAG_INIT(this->exit);
}

void Queue::enq(ThreadContext &ctx, Data d)
{
    Node *node = new Node(d);
    while (true) {
        Node *tail = LL(&this->Tail, &ctx.myver, &ctx.mynode);
        node->pred = tail;
        if (CAS((uint64_t*)&tail->next, (uint64_t)NULL, (uint64_t)node)) {
            SC(&this->Tail, node, ctx.myver, ctx.mynode);
            break;
        } else {
            SC(&this->Tail, tail->next, ctx.myver, ctx.mynode);
        }
    }
}

Data Queue::deq(ThreadContext &ctx)
{
    while (true) {
        Node *head = LL(&this->Head, &ctx.myver, &ctx.mynode);
        Node *next = head->next;
        if (next == NULL) {
            Unlink(&this->Head, ctx.myver, ctx.mynode);
            return 0;
        }
        if (SC(&this->Head, next, ctx.myver, ctx.mynode)) {
            Data d = next->d;
            SetToBeFreed(next);
            return d;
        }
    }
}

Queue::Queue(void)
{
    this->Tail.entry.ver = 0;
    this->Tail.entry.count = 0;

    this->Tail.ptr0 = new Node(0);
    this->Tail.ptr1 = new Node(0);

    this->Tail.ptr0->pred = this->Tail.ptr1;
    EXITTAG_INIT(this->Tail.ptr0->exit);
    this->Tail.ptr0->next = NULL;
    this->Tail.ptr1->exit.count = 0;
    this->Tail.ptr1->exit.outstandingTransfers = 0;
    this->Tail.ptr1->exit.nlP = 0;
    this->Tail.ptr1->exit.toBeFreed = 0;

    this->Head = this->Tail;
}

void Queue::dump() const
{
    const Loc *loc = &this->Head;
    Node *cur = CURRENT(loc, loc->entry.ver);
    while ((cur = cur->next) != NULL) {
        fprintf(stderr, "[%ld]", cur->d);
    }
    fprintf(stderr, "\n");
}

bool Queue::isEmpty() const
{
    const Loc *loc = &this->Head;
    Node *cur = CURRENT(loc, loc->entry.ver);
    return cur->next == NULL;
}
} /* namespace logpool */

//#include <assert.h>
//using namespace logpool;
//Queue *q;
//int main(int argc, char const* argv[])
//{
//    int i, max = 10;
//    assert(sizeof(EntryTag) <= sizeof(uint64_t));
//    assert(sizeof(ExitTag) <= sizeof(uint64_t));
//    if (!q) {
//        q = new Queue();
//    }
//
//    ThreadContext ctx = {0, NULL};
//    for (i = 0; i < max; i++) {
//        q->enq(ctx, i);
//        q->dump();
//    }
//
//    for (i = 0; i < max; i++) {
//        Data d = q->deq(ctx);
//        assert(i == d);
//        q->dump();
//    }
//    return 0;
//}
