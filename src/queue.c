#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}


#ifdef MLQ_SCHED
void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if (q == NULL || proc == NULL) return;

        if (q->size >= MAX_QUEUE_SIZE){
                printf("Queue is full\n");
                return; 
        }

        if (q->size < 0){
                printf("Wrong size of queue list\n");
                return;
        }

        
        q->proc[q->size] = proc;
        (q->size)++;

        // int i;
        // for (i = 0; i < q->size; i++){
        //         if (proc->priority < q->proc[i]->priority) {
        //                 break;
        //         }
        // }

        // for (int j = q->size; j > i ; j--){
        //         q->proc[j]= q->proc[j-1];
        // }

        // q->proc[i] = proc; 
        // q->size++;

        return;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */

         if (q->size == 0 || q == NULL){
                printf("Queue is empty\n");
                return NULL;
         }
         
        // struct pcb_t * pcb_t_ptr =  q->proc[0];

        //  for (int i=0; i<q->size; i++){
        //         q->proc[i] = q->proc[i+1];
        //  }

        //  (q->size)--;

        int highest_prio_indx = 0; // (lowest priority number = highest priority)
        for (int i = 1; i< q->size ; i++){
                if (q->proc[i]->priority < q->proc[highest_prio_indx]){
                        highest_prio_indx = i;
                }
        }

        struct pcb_t * pcb_t_ptr = q->proc[highest_prio_indx];

        for (int i = highest_prio_indx; i<q->size-1; i++){
                q->proc[i] = q->proc[i+1];
        }
        q->size--;


        // if ( q == NULL || q->size ==0){
        //      printf ("Queue is empty\n"); 
        //      return NULL;  
        // }

        // struct pcb_t * proc = q->proc[0];

	return pcb_t_ptr;
}

#endif