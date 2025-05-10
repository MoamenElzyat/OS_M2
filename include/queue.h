#ifndef QUEUE_H
#define QUEUE_H

#include "pcb.h"
#include "scheduler.h"



void init_queue(ProcessQueue* queue);
void free_queue(ProcessQueue* queue);
void enqueue(ProcessQueue* queue, PCB* process);
PCB* dequeue(ProcessQueue* queue);
PCB* peek(ProcessQueue* queue);
int is_empty(ProcessQueue* queue);
void print_queue(ProcessQueue* queue);
PCB* remove_from_queue(ProcessQueue* queue, int index);
void add_to_queue(ProcessQueue* queue, PCB* pcb);

#endif // QUEUE_H