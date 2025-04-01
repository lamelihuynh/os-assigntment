#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t *q) {
    return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc) {
    if (q == NULL || proc == NULL || q->size >= MAX_QUEUE_SIZE) {
        return;
    }

    int i = q->size - 1;
    
    while (i >= 0 && q->proc[i] != NULL && q->proc[i]->priority < proc->priority) {
        q->proc[i + 1] = q->proc[i];
        i--;
    }
    
    q->proc[i + 1] = proc;
    q->size++;
}

struct pcb_t *dequeue(struct queue_t *q) {
    if (q == NULL || empty(q)) {
        return NULL;
    }

    struct pcb_t *proc = q->proc[0];

    for (int i = 1; i < q->size; i++) {
        q->proc[i - 1] = q->proc[i];
    }

    q->proc[q->size - 1] = NULL;  // Xoá con trỏ cuối cùng
    q->size--;

    return proc;
}
