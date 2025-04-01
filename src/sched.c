
#include "queue.h"
#include "sched.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

static struct queue_t running_list;
#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int slot[MAX_PRIO];
#endif

int queue_empty(void) {
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if(!empty(&mlq_ready_queue[prio])) 
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
#ifdef MLQ_SCHED
    int i ;

	for (i = 0; i < MAX_PRIO; i ++) {
		mlq_ready_queue[i].size = 0;
		slot[i] = MAX_PRIO - i; 
	}
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED
/* 
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t * get_mlq_proc(void) {
	/*TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	struct pcb_t * proc = NULL;
    /*Get a process from PRIORITY [ready_queue]*/
    pthread_mutex_lock(&queue_lock);
    
    // Static variable to keep track of the current priority level being served
    static unsigned long current_prio = 0;
    // Static array to keep track of time slots used for each priority
    static int current_slot[MAX_PRIO] = {0};
    
    // Find a non-empty queue starting from the highest priority
    int found = 0;
    for (int i = 0; i < MAX_PRIO && !found; i++) {
        // Calculate the actual priority level to check, starting from current_prio
        unsigned long prio = (current_prio + i) % MAX_PRIO;
        
        if (!empty(&mlq_ready_queue[prio])) {
            // We found a non-empty queue
            found = 1;
            
            // If we're still within the allocated slot for this priority
            if (current_slot[prio] < slot[prio]) {
                proc = dequeue(&mlq_ready_queue[prio]);
                current_slot[prio]++;
                
                // Set current_prio to the priority we just served
                current_prio = prio;
            } else {
                // Reset the slot counter and move to next priority
                current_slot[prio] = 0;
                current_prio = (prio + 1) % MAX_PRIO;
                // Try again in the next iteration
                found = 0;
            }
        }
    }
    
    // If we went through all priorities and didn't find anything, reset all slots
    if (proc == NULL) {
        for (int i = 0; i < MAX_PRIO; i++) {
            current_slot[i] = 0;
        }
    }
    
    pthread_mutex_unlock(&queue_lock);
    return proc;
}

void put_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);	
}

struct pcb_t * get_proc(void) {
	return get_mlq_proc();
}

void put_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
    proc->mlq_ready_queue = mlq_ready_queue;
    proc->running_list = &running_list;

    /* Put running proc to running_list */
    pthread_mutex_lock(&queue_lock);
    enqueue(proc->running_list, proc);
    pthread_mutex_unlock(&queue_lock);

    return put_mlq_proc(proc);
}

void add_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
    proc->mlq_ready_queue = mlq_ready_queue;
    proc->running_list = &running_list;

    /* Put running proc to running_list */
    pthread_mutex_lock(&queue_lock);
    enqueue(proc->running_list, proc);
    pthread_mutex_unlock(&queue_lock);

    return add_mlq_proc(proc);
}
#else
struct pcb_t * get_proc(void) {
	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	struct pcb_t * proc = NULL;
    pthread_mutex_lock(&queue_lock);
    
    if (!empty(&ready_queue)) {
        proc = dequeue(&ready_queue);
    } else if (!empty(&run_queue)) {
        proc = dequeue(&run_queue);
    }
    
    pthread_mutex_unlock(&queue_lock);
    return proc;
}

void put_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
    proc->running_list = &running_list;

    /* TODO: put running proc to running_list */
    pthread_mutex_lock(&queue_lock);
    enqueue(proc->running_list, proc);
    pthread_mutex_unlock(&queue_lock);

    pthread_mutex_lock(&queue_lock);
    enqueue(&run_queue, proc);
    pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
    proc->running_list = &running_list;

    /* TODO: put running proc to running_list */
    pthread_mutex_lock(&queue_lock);
    enqueue(proc->running_list, proc);
    pthread_mutex_unlock(&queue_lock);

    pthread_mutex_lock(&queue_lock);
    enqueue(&ready_queue, proc);
    pthread_mutex_unlock(&queue_lock); 	
}
#endif


