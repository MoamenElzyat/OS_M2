#ifndef QUEUE_H
#define QUEUE_H

#include "pcb.h"

typedef struct {
    PCB** processes;
    int size;
    int capacity;
} SimpleQueue;

// Queue operations
void init_queue(SimpleQueue* queue);
void free_queue(SimpleQueue* queue);
void enqueue(SimpleQueue* queue, PCB* process);
PCB* dequeue(SimpleQueue* queue);
PCB* peek(SimpleQueue* queue);
int is_empty(SimpleQueue* queue);
void print_queue(SimpleQueue* queue);

#endif // QUEUE_H