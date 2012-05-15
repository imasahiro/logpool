#include "lfqueue.h"
#include "sched.h"
#include <pthread.h>

namespace logpool {

class Task {
public:
    virtual void Run() = 0;
    bool *join;
#ifdef DEBUG
    virtual void dump() { fprintf(stderr, "{Task %p}\n", this); }
#endif
};

class JoinTask : Task {
    void Run() { *join = true; }
#ifdef DEBUG
    virtual void dump() { fprintf(stderr, "{Join %p}\n", this); }
#endif
};

struct Scheduler {
public:
    Scheduler(size_t num_workers);
    ~Scheduler();
    void enqueue(Task *task);
    void join();

private:
    struct Worker {
        pthread_t thread;
        bool join_;
        Queue *q_;
        Worker(Queue *q) : join_(false), q_(q) {}
        static void *exec(void *arg);
    };
    ThreadContext ctx;
    size_t active_workers;
    Queue *q;
    std::vector<Worker*> workers;
};

Scheduler::Scheduler(size_t num_workers) : active_workers(0)
{
    q = new Queue();
    for (size_t i = 0; i < num_workers; ++i) {
        Worker *w = new Worker(q);
        pthread_create(&w->thread, NULL, Worker::exec, w);
        workers.push_back(w);
        ++active_workers;
    }
}

Scheduler::~Scheduler()
{
    if (active_workers) {
        join();
    }
}

void Scheduler::enqueue(Task *task)
{
    q->enq(ctx, reinterpret_cast<Data>(task));
}

void Scheduler::join() {
    for (size_t i = 0; i < active_workers; ++i) {
        JoinTask *task = new JoinTask();
        q->enq(ctx, reinterpret_cast<Data>(task));
    }
    for (size_t i = 0; i < active_workers; ++i) {
        Worker *w = workers[i];
        pthread_join(w->thread, NULL);
        delete w;
        workers[i] = NULL;
    }
    active_workers = 0;
    workers.clear();
}
void *Scheduler::Worker::exec(void *arg)
{
    Worker *w = reinterpret_cast<Worker*>(arg);
    ThreadContext ctx = {0, NULL};
    while (!w->join_) {
        if (Task *t = reinterpret_cast<Task*>(w->q_->deq(ctx))) {
            t->join = &w->join_;
#ifdef DEBUG
            t->dump();
#endif
            t->Run();
            delete t;
        }
    }
    return NULL;
}

} /* namespace logpool */

using namespace logpool;

class MyTask : public Task {
    void Run() {}
#ifdef DEBUG
    virtual void dump() { fprintf(stderr, "{MyTask %p}\n", this); }
#endif
};

#define MAX 20000000
#define CORE 4
int main(int argc, char const* argv[])
{
    Scheduler sched(CORE);
    for (int i = 0; i < MAX; ++i) {
        sched.enqueue(new MyTask());
    }
    return 0;
}
