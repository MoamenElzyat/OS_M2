#include "../include/queue.h"
#include <stdlib.h>
#include <stdio.h>

#define INITIAL_CAPACITY 10

void init_queue(Queue* queue) {
    queue->processes = (PCB**)malloc(INITIAL_CAPACITY * sizeof(PCB*));
    queue->size = 0;
    queue->capacity = INITIAL_CAPACITY;
}

void free_queue(Queue* queue) {
    free(queue->processes);
    queue->processes = NULL;
    queue->size = 0;
    queue->capacity = 0;
}

void enqueue(Queue* queue, PCB* process) {
    if (queue->size >= queue->capacity) {
        queue->capacity *= 2;
        queue->processes = (PCB**)realloc(queue->processes, queue->capacity * sizeof(PCB*));
    }
    queue->processes[queue->size++] = process;
}

PCB* dequeue(Queue* queue) {
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

PCB* peek(Queue* queue) {
    if (queue->size == 0) {
        return NULL;
    }
    return queue->processes[0];
}

int is_empty(Queue* queue) {
    return queue->size == 0;
}

void print_queue(Queue* queue) {
    printf("Queue contents (%d processes):\n", queue->size);
    for (int i = 0; i < queue->size; i++) {
        PCB* p = queue->processes[i];
        printf("  PID: %d, State: %d, Priority: %d, PC: %d\n",
               p->pid, p->state, p->priority, p->program_counter);
    }
} 