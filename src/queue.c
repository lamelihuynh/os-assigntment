#include <stdio.h>
#include <stdlib.h>
#include "queue.h"


int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if( q == NULL) {
                fprintf(stderr, "Error: queue is NULL\n");
                return;
        }
        if( proc == NULL) {
                fprintf(stderr, "Error: process is NULL\n");
                return;
        }
        if( q->size < MAX_QUEUE_SIZE) {
                q->proc[q->size] = proc;
                proc->ready_queue = q;
                q->size++;
        } else {
                fprintf(stderr, "Error: queue is full\n");             
        }
        return;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        
        if( q == NULL){
                fprintf(stderr, "Error: queue is NULL\n");
                return NULL;
         }

        if( q->size == 0){
                fprintf(stderr, "Error: queue is empty\n");
                return NULL;
        }
        // Find the process with the highest priority
        int prio_i = 0;
        for( int i = 1; i < q->size; i++){
                if( q->proc[i]->prio < q->proc[prio_i]->prio){
                        prio_i = i;
                }
        }

        struct pcb_t * ans = q->proc[prio_i];
        // Remove the process from the queue
        for( int i = prio_i; i < q->size - 1; i++){
                q->proc[i] = q->proc[i + 1];
        }
        q->size--;
        q->proc[q->size] = NULL;
        ans->ready_queue = NULL;
        return ans;

}
