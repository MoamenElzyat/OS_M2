#include "../include/queue.h"
#include <stdlib.h>
#include <stdio.h>
#include "globals.h" 
#include "scheduler.h"  
#define INITIAL_CAPACITY 10

void init_queue(ProcessQueue* queue) {
    queue->processes = (PCB**)malloc(INITIAL_CAPACITY * sizeof(PCB*));
    queue->size = 0;
    queue->capacity = INITIAL_CAPACITY;
}

void free_queue(ProcessQueue* queue) {
    free(queue->processes);
    queue->processes = NULL;
    queue->size = 0;
    queue->capacity = 0;
}

void enqueue(ProcessQueue* queue, PCB* process) {
    if (queue->size >= queue->capacity) {
        queue->capacity *= 2;
        queue->processes = (PCB**)realloc(queue->processes, queue->capacity * sizeof(PCB*));
    }
    queue->processes[queue->size++] = process;
}

PCB* dequeue(ProcessQueue* queue) {
    if (queue->size == 0) {
        return NULL;
    }
    PCB* process = queue->processes[0];
    for (int i = 1; i < queue->size; i++) {
        queue->processes[i-1] = queue->processes[i];
    }
    queue->size--;
    return process;
}

PCB* peek(ProcessQueue* queue) {
    if (queue->size == 0) {
        return NULL;
    }
    return queue->processes[0];
}

int is_empty(ProcessQueue* queue) {
    return queue->size == 0;
}

void print_queue(ProcessQueue* queue) {
    printf("Queue contents (%d processes):\n", queue->size);
    for (int i = 0; i < queue->size; i++) {
        PCB* p = queue->processes[i];
        printf("  PID: %d, State: %d, Priority: %d, PC: %d\n",
            p->pid, p->state, p->priority, p->program_counter);
    }
}


