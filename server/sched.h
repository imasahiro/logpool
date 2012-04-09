#include <vector>

#ifndef SCHED_H_
#define SCHED_H_
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
} /* namespace logpool */

#endif /* end of include guard */
