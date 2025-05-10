#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pcb.h"

// Constants
#define QUANTUM 10
#define MAX_QUEUES 5
#define MAX_PROCESSES 100

// ProcessQueue structure 
typedef struct {
    PCB** processes;
    int size;
    int capacity;
} ProcessQueue;

// Scheduling algorithms
typedef enum {
    FCFS,           // First Come First Serve
    RR,             // Round Robin
    MLFQ            // Multilevel Feedback Queue
} SchedulingAlgorithm;

// Scheduler structure ✅
typedef struct {
    SchedulingAlgorithm algorithm;
    int quantum;                   // For RR and MLFQ
    ProcessQueue ready_queues[4];  // For MLFQ (4 priority levels)
    ProcessQueue blocked_queue;
    PCB* running_process;
    int clock_cycle;
    int next_pid;   
    int initialized;               
} Scheduler;

typedef struct {
    PCB* list[MAX_PROCESSES];
    int count;
} PendingList;

extern PendingList pending_list;

// Function declarations
void init_scheduler(Scheduler* scheduler, SchedulingAlgorithm algorithm, int quantum);
void add_process(Scheduler* scheduler, PCB* pcb);
PCB* schedule_next_process(Scheduler* scheduler);
void update_scheduler(Scheduler* scheduler);
void print_scheduler_status(const Scheduler* scheduler);
void destroy_scheduler(Scheduler* scheduler);
PCB* create_process(const char* program_name, int priority);
void schedule();
void run_process(PCB* pcb);
void context_switch(PCB* current, PCB* next);
void update_queues();
void promote_process(PCB* pcb);
void demote_process(PCB* pcb);
void scheduler_step();
bool is_all_queues_empty(Scheduler* scheduler);
void print_queues_state(Scheduler* scheduler);
bool is_in_blocked_queue(Scheduler* scheduler, PCB* pcb);




#endif // SCHEDULER_H