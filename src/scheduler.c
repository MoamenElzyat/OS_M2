#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"

#define INITIAL_QUEUE_CAPACITY 10

// Initialize a process queue
static void init_process_queue(ProcessQueue* queue) {
    queue->size = 0;
    queue->capacity = INITIAL_QUEUE_CAPACITY;
    queue->processes = malloc(INITIAL_QUEUE_CAPACITY * sizeof(PCB*));
}

// Add a process to a queue
static void add_to_queue(ProcessQueue* queue, PCB* pcb) {
    if (queue->size >= queue->capacity) {
        queue->capacity *= 2;
        queue->processes = realloc(queue->processes, queue->capacity * sizeof(PCB*));
    }
    queue->processes[queue->size++] = pcb;
}

// Remove a process from a queue
static PCB* remove_from_queue(ProcessQueue* queue, int index) {
    if (index < 0 || index >= queue->size) return NULL;
    PCB* pcb = queue->processes[index];
    for (int i = index; i < queue->size - 1; i++) {
        queue->processes[i] = queue->processes[i + 1];
    }
    queue->size--;
    return pcb;
}

// Initialize scheduler
void init_scheduler(Scheduler* scheduler, SchedulingAlgorithm algorithm, int quantum) {
    scheduler->algorithm = algorithm;
    scheduler->quantum = quantum;
    scheduler->running_process = NULL;
    scheduler->clock_cycle = 0;
    scheduler->next_pid = 1;

    // Initialize queues
    for (int i = 0; i < 4; i++) {
        init_process_queue(&scheduler->ready_queues[i]);
    }
    init_process_queue(&scheduler->blocked_queue);
}

// Add a process to the scheduler
void add_process(Scheduler* scheduler, PCB* pcb) {
    if (!scheduler || !pcb) return;

    set_pcb_state(pcb, READY);
    if (scheduler->algorithm == MLFQ) {
        add_to_queue(&scheduler->ready_queues[pcb->priority - 1], pcb);
    } else {
        add_to_queue(&scheduler->ready_queues[0], pcb);
    }
}

// Schedule next process based on algorithm
PCB* schedule_next_process(Scheduler* scheduler) {
    if (!scheduler) return NULL;

    PCB* next_process = NULL;
    switch (scheduler->algorithm) {
        case FCFS:
            if (scheduler->ready_queues[0].size > 0) {
                next_process = remove_from_queue(&scheduler->ready_queues[0], 0);
            }
            break;
        case RR:
            if (scheduler->ready_queues[0].size > 0) {
                next_process = remove_from_queue(&scheduler->ready_queues[0], 0);
                next_process->quantum_remaining = scheduler->quantum;
            }
            break;
        case MLFQ:
            for (int i = 0; i < 4; i++) {
                if (scheduler->ready_queues[i].size > 0) {
                    next_process = remove_from_queue(&scheduler->ready_queues[i], 0);
                    next_process->quantum_remaining = (1 << i);  // 2^i
                    break;
                }
            }
            break;
    }
    if (next_process) {
        set_pcb_state(next_process, RUNNING);
    }
    return next_process;
}

// Update scheduler state
void update_scheduler(Scheduler* scheduler) {
    if (!scheduler) return;
    scheduler->clock_cycle++;

    if (scheduler->running_process) {
        if (scheduler->algorithm != FCFS) {
            scheduler->running_process->quantum_remaining--;
            if (scheduler->running_process->quantum_remaining <= 0) {
                set_pcb_state(scheduler->running_process, READY);
                if (scheduler->algorithm == MLFQ) {
                    int current_priority = scheduler->running_process->priority;
                    if (current_priority < 4) {
                        set_pcb_priority(scheduler->running_process, current_priority + 1);
                    }
                }
                add_process(scheduler, scheduler->running_process);
                scheduler->running_process = NULL;
            }
        }
    }

    if (!scheduler->running_process) {
        scheduler->running_process = schedule_next_process(scheduler);
    }
}

// Debugging status printer
void print_scheduler_status(const Scheduler* scheduler) {
    if (!scheduler) return;

    printf("\n==================== Scheduler Debug ====================\n");
    printf("Clock Cycle: %d\n", scheduler->clock_cycle);
    printf("Algorithm: %s\n", 
           scheduler->algorithm == FCFS ? "FCFS" : 
           scheduler->algorithm == RR ? "Round Robin" : "MLFQ");

    if (scheduler->running_process) {
        printf("🟢 Running Process: PID %d | Priority: %d | PC: %d\n",
               scheduler->running_process->pid,
               scheduler->running_process->priority,
               scheduler->running_process->program_counter);
    } else {
        printf("🟠 No process is currently running.\n");
    }

    printf("\n🔎 Ready Queues:\n");
    for (int i = 0; i < 4; i++) {
        printf("  Priority %d (%d processes): ", i + 1, scheduler->ready_queues[i].size);
        for (int j = 0; j < scheduler->ready_queues[i].size; j++) {
            PCB* p = scheduler->ready_queues[i].processes[j];
            printf("[PID %d] ", p->pid);
        }
        printf("\n");
    }

    printf("\n🔒 Blocked Queue (%d processes): ", scheduler->blocked_queue.size);
    for (int j = 0; j < scheduler->blocked_queue.size; j++) {
        PCB* p = scheduler->blocked_queue.processes[j];
        printf("[PID %d] ", p->pid);
    }
    printf("\n========================================================\n\n");
}

// Destroy scheduler
void destroy_scheduler(Scheduler* scheduler) {
    if (!scheduler) return;
    for (int i = 0; i < 4; i++) {
        free(scheduler->ready_queues[i].processes);
    }
    free(scheduler->blocked_queue.processes);
}