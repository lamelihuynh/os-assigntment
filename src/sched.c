#include "queue.h"
#include "sched.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
#endif

int queue_empty(void) {
    pthread_mutex_lock(&queue_lock);
    int is_empty = empty(&ready_queue) && empty(&run_queue);

#ifdef MLQ_SCHED
    for (int i = 0; i < MAX_PRIO; i++) {
        if (!empty(&mlq_ready_queue[i])) {
            is_empty = 0;
            break;
        }
    }
#endif

    pthread_mutex_unlock(&queue_lock);
    return is_empty;
}

void init_scheduler(void) {
    ready_queue.size = 0;
    run_queue.size = 0;
    pthread_mutex_init(&queue_lock, NULL);

#ifdef MLQ_SCHED
    for (int i = 0; i < MAX_PRIO; i++) {
        mlq_ready_queue[i].size = 0;
    }
#endif
}

#ifdef MLQ_SCHED
struct pcb_t * get_mlq_proc(void) {
    struct pcb_t * proc = NULL;

    for (int prio = 0; prio < MAX_PRIO; prio++) {
        if (!empty(&mlq_ready_queue[prio])) {
            proc = dequeue(&mlq_ready_queue[prio]);
            return proc;
        }
    }

    return NULL;
}

void put_mlq_proc(struct pcb_t * proc) {
    if (proc->priority >= 0 && proc->priority < MAX_PRIO) {
        enqueue(&mlq_ready_queue[proc->priority], proc);
    }
}

void add_mlq_proc(struct pcb_t * proc) {
    if (proc->priority >= 0 && proc->priority < MAX_PRIO) {
        enqueue(&mlq_ready_queue[proc->priority], proc);
    }
}
#endif

struct pcb_t * get_proc(void) {
    pthread_mutex_lock(&queue_lock);
    struct pcb_t * proc = dequeue(&ready_queue);
    pthread_mutex_unlock(&queue_lock);

#ifdef MLQ_SCHED
    if (!proc) {
        pthread_mutex_lock(&queue_lock);
        proc = get_mlq_proc();
        pthread_mutex_unlock(&queue_lock);
    }
#endif

    return proc;
}

void put_proc(struct pcb_t * proc) {
    pthread_mutex_lock(&queue_lock);
    enqueue(&run_queue, proc);
    pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
    pthread_mutex_lock(&queue_lock);

#ifdef MLQ_SCHED
    add_mlq_proc(proc);
#else
    enqueue(&ready_queue, proc);
#endif

    pthread_mutex_unlock(&queue_lock);
}
