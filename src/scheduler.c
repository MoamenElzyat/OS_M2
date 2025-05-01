#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"
#include "../include/queue.h"

// Global variable definitions
int next_pid = 1;
int current_time = 0;
SchedulingAlgorithm scheduling_algorithm = FCFS;
Queue ready_queue;
Queue ready_queues[MAX_QUEUES];

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
    scheduler->next_pid = 1;  // Start PIDs at 1

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
        // Add to appropriate priority queue
        add_to_queue(&scheduler->ready_queues[pcb->priority - 1], pcb);
    } else {
        // Add to first ready queue for FCFS and RR
        add_to_queue(&scheduler->ready_queues[0], pcb);
    }
}

// Schedule next process based on algorithm
PCB* schedule_next_process(Scheduler* scheduler) {
    if (!scheduler) return NULL;

    PCB* next_process = NULL;

    switch (scheduler->algorithm) {
        case FCFS:
            // First Come First Serve
            if (scheduler->ready_queues[0].size > 0) {
                next_process = remove_from_queue(&scheduler->ready_queues[0], 0);
            }
            break;

        case RR:
            // Round Robin
            if (scheduler->ready_queues[0].size > 0) {
                next_process = remove_from_queue(&scheduler->ready_queues[0], 0);
                next_process->quantum_remaining = scheduler->quantum;
            }
            break;

        case MLFQ:
            // Multilevel Feedback Queue
            for (int i = 0; i < 4; i++) {
                if (scheduler->ready_queues[i].size > 0) {
                    next_process = remove_from_queue(&scheduler->ready_queues[i], 0);
                    next_process->quantum_remaining = (1 << i);  // Quantum doubles with each level
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
        // Update quantum for RR and MLFQ
        if (scheduler->algorithm != FCFS) {
            scheduler->running_process->quantum_remaining--;
            
            // Check if quantum expired
            if (scheduler->running_process->quantum_remaining <= 0) {
                set_pcb_state(scheduler->running_process, READY);
                
                if (scheduler->algorithm == MLFQ) {
                    // Demote process to lower priority queue
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

    // Check if we need to schedule a new process
    if (!scheduler->running_process) {
        scheduler->running_process = schedule_next_process(scheduler);
    }
}

// Print scheduler status
void print_scheduler_status(const Scheduler* scheduler) {
    if (!scheduler) return;

    printf("Scheduler Status (Clock Cycle: %d):\n", scheduler->clock_cycle);
    printf("Algorithm: %s\n", 
           scheduler->algorithm == FCFS ? "FCFS" : 
           scheduler->algorithm == RR ? "Round Robin" : "MLFQ");
    
    if (scheduler->running_process) {
        printf("Running Process: PID %d\n", scheduler->running_process->pid);
    } else {
        printf("No process running\n");
    }

    printf("\nReady Queues:\n");
    for (int i = 0; i < 4; i++) {
        printf("Priority %d: %d processes\n", i + 1, scheduler->ready_queues[i].size);
    }

    printf("\nBlocked Queue: %d processes\n", scheduler->blocked_queue.size);
}

// Destroy scheduler
void destroy_scheduler(Scheduler* scheduler) {
    if (!scheduler) return;

    // Free all queues
    for (int i = 0; i < 4; i++) {
        free(scheduler->ready_queues[i].processes);
    }
    free(scheduler->blocked_queue.processes);
}

PCB* create_process(const char* program_name, int priority) {
    PCB* pcb = (PCB*)malloc(sizeof(PCB));
    if (!pcb) {
        return NULL;
    }

    // Initialize PCB fields
    pcb->pid = next_pid++;
    strncpy(pcb->program_name, program_name, MAX_PROGRAM_NAME_LENGTH - 1);
    pcb->program_name[MAX_PROGRAM_NAME_LENGTH - 1] = '\0';
    pcb->state = READY;
    pcb->priority = priority;
    pcb->program_counter = 0;
    pcb->quantum_remaining = QUANTUM;
    pcb->arrival_time = current_time;
    pcb->time_in_queue = 0;
    pcb->memory_lower_bound = -1;
    pcb->memory_upper_bound = -1;
    pcb->variables = NULL;
    pcb->var_count = 0;
    pcb->instructions = NULL;
    pcb->instruction_count = 0;

    // Add to appropriate queue based on scheduling algorithm
    if (scheduling_algorithm == FCFS) {
        enqueue(&ready_queue, pcb);
    } else if (scheduling_algorithm == RR) {
        enqueue(&ready_queue, pcb);
    } else if (scheduling_algorithm == MLFQ) {
        enqueue(&ready_queues[priority - 1], pcb);
    }

    return pcb;
} 