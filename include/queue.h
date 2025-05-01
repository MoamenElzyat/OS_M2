#ifndef QUEUE_H
#define QUEUE_H

#include "scheduler.h"

// Queue operations
void init_queue(Queue* queue);
void free_queue(Queue* queue);
void enqueue(Queue* queue, PCB* process);
PCB* dequeue(Queue* queue);
PCB* peek(Queue* queue);
int is_empty(Queue* queue);
void print_queue(Queue* queue);

#endif // QUEUE_H 