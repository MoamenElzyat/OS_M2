#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"
#include "../include/globals.h"  
#include "../include/interpreter.h"
#include "../include/memory.h"
#include "../include/mutex.h"
#include "../include/pcb.h"
#include "../include/queue.h"

#define INITIAL_QUEUE_CAPACITY 10

PendingList pending_list = {.count = 0};

// Initialize a process queue
static void init_process_queue(ProcessQueue* queue) {
    queue->size = 0;
    queue->capacity = INITIAL_QUEUE_CAPACITY;
    queue->processes = malloc(INITIAL_QUEUE_CAPACITY * sizeof(PCB*));
}

// Add a process to a queue
void add_to_queue(ProcessQueue* queue, PCB* pcb) {
    if (!pcb) {
        printf("[ERROR] Tried to add NULL PCB to queue!\n");
        return;
    }
    if (queue->size >= queue->capacity) {
        queue->capacity *= 2;
        queue->processes = realloc(queue->processes, queue->capacity * sizeof(PCB*));
    }
    queue->processes[queue->size++] = pcb;
}

// Remove a process from a queue
PCB* remove_from_queue(ProcessQueue* queue, int index) {
    if (index < 0 || index >= queue->size) {
        printf("[ERROR] remove_from_queue: invalid index %d (size: %d)\n", index, queue->size);
        return NULL;
    }

    if (!queue->processes) {
        printf("[FATAL ERROR] queue->processes is NULL!!! (queue size: %d)\n", queue->size);
        return NULL;
    }

    if (!queue->processes[index]) {
        printf("[FATAL ERROR] queue->processes[%d] is NULL!!! (queue size: %d)\n", index, queue->size);
        return NULL;
    }

    PCB* pcb = queue->processes[index];  
    
    printf("[DEBUG] remove_from_queue: removing PID %d from index %d (queue size before: %d)\n",
            pcb->pid, index, queue->size);  
    
    for (int i = index; i < queue->size - 1; i++) {
        queue->processes[i] = queue->processes[i + 1];
    }
    queue->size--;
    printf("[DEBUG] Queue state after removal (size=%d)\n", queue->size);
    return pcb;
}

// Initialize scheduler
void init_scheduler(Scheduler* scheduler, SchedulingAlgorithm algorithm, int quantum) {
    printf("[DEBUG C] Inside init_scheduler: setting algorithm to %d\n", algorithm);
    scheduler->algorithm = algorithm;
    scheduler->quantum = quantum;
    scheduler->running_process = NULL;
    scheduler->clock_cycle = 0;
    printf("[INIT] Scheduler initialized with Clock Cycle = %d\n", scheduler->clock_cycle);
    scheduler->next_pid = 1;
    scheduler->initialized = 1;

    for (int i = 0; i < 4; i++) {
        init_process_queue(&scheduler->ready_queues[i]);
        printf("[TRACE] init_scheduler: initialized ready_queues[%d] => processes=%p\n",
            i, scheduler->ready_queues[i].processes);
    }
    init_process_queue(&scheduler->blocked_queue);
    print_queues_state(scheduler);
}

// Add a process to the scheduler
void add_process(Scheduler* scheduler, PCB* pcb) {
    if (!scheduler || !pcb) {
        printf("[ERROR] add_process called with NULL scheduler or pcb!\n");
        return;
    }
    print_queues_state(scheduler);
    printf("[TRACE] add_process: priority=%d, ready_queues[%d].processes=%p\n",
        pcb->priority,
        scheduler->algorithm == MLFQ ? pcb->priority - 1 : 0,
        scheduler->algorithm == MLFQ ? scheduler->ready_queues[pcb->priority - 1].processes 
                                        : scheduler->ready_queues[0].processes);

    // Handle pending processes (future arrivals)
    if (pcb->arrival_time > scheduler->clock_cycle) {
        printf("[CHECK] Adding PID %d to Pending List (Arrival: %d, Clock: %d)\n", pcb->pid, pcb->arrival_time, scheduler->clock_cycle);
        pending_list.list[pending_list.count++] = pcb;
        return;
    } else {
        printf("[CHECK] Adding PID %d DIRECTLY to Ready Queue (Arrival: %d, Clock: %d)\n", pcb->pid, pcb->arrival_time, scheduler->clock_cycle);
    }

    set_pcb_state(pcb, READY);

    printf("[DEBUG] ✅✅ Added PID %d to READY queue (Priority: %d)\n", pcb->pid, pcb->priority);
    print_scheduler_status(scheduler);

    // Check priority before adding to MLFQ
    int priority = pcb->priority;
    if (scheduler->algorithm == MLFQ) {

        if (priority < 1) {
            printf("[WARN] PCB PID %d had priority < 1 (was %d), fixing to 1.\n", pcb->pid, priority);
            priority = 1;
        } else if (priority > 4) {
            printf("[WARN] PCB PID %d had priority > 4 (was %d), fixing to 4.\n", pcb->pid, priority);
            priority = 4;
        }

        if (!scheduler->ready_queues[priority - 1].processes) {
            printf("[FATAL ERROR] ready_queues[%d] processes is NULL!\n", priority - 1);
            return;
        }

        ProcessQueue* queue = &scheduler->ready_queues[priority - 1];
        bool already_in_queue = false;
        for (int i = 0; i < queue->size; i++) {
            if (queue->processes[i] == pcb) {
                already_in_queue = true;
                break;
            }
        }

        if (!already_in_queue) {
            add_to_queue(queue, pcb);
            printf("[DEBUG] Added PID %d to MLFQ ready queue (Priority: %d, Arrival: %d)\n", pcb->pid, priority, pcb->arrival_time);
        } else {
            printf("[DEBUG] Skipping add: PID %d is already in MLFQ ready queue (Priority: %d)\n", pcb->pid, priority);
        }

    } else {
        if (!scheduler->ready_queues[0].processes) {
            printf("[FATAL ERROR] ready_queues[0] processes is NULL!\n");
            return;
        }

        ProcessQueue* queue = &scheduler->ready_queues[0];
        bool already_in_queue = false;
        for (int i = 0; i < queue->size; i++) {
            if (queue->processes[i] == pcb) {
                already_in_queue = true;
                break;
            }
        }

        if (!already_in_queue) {
            add_to_queue(queue, pcb);
            printf("[DEBUG] Added PID %d to ready queue (Arrival: %d)\n", pcb->pid, pcb->arrival_time);
        } else {
            printf("[DEBUG] Skipping add: PID %d is already in ready queue\n", pcb->pid);
        }
    }
}

// Schedule next process based on algorithm
PCB* schedule_next_process(Scheduler* scheduler) {
    printf("[DEBUG] Scheduling Algorithm: %d (0=FCFS,1=RR,2=MLFQ)\n", scheduler->algorithm);
    printf("[DEBUG] Checking ready queues:\n");
    for (int i = 0; i < 4; i++) {
        printf("Priority %d: %d processes\n", i + 1, scheduler->ready_queues[i].size);
    }
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
                if (next_process) {
                    next_process->quantum_remaining = scheduler->quantum;
                    printf("[Round Robin] Scheduled PID %d with quantum %d\n",
                        next_process->pid, next_process->quantum_remaining);
                } else {
                    printf("[ERROR] remove_from_queue returned NULL in RR!\n");
                }
            }
            break;
        case MLFQ:
            for (int i = 0; i < 4; i++) {
                if (scheduler->ready_queues[i].size > 0) {
                    next_process = remove_from_queue(&scheduler->ready_queues[i], 0);
                    if (next_process) {
                        printf("[MLFQ] Scheduled PID %d from Priority %d\n", next_process->pid, i + 1);
                        next_process->quantum_remaining = (1 << i);  
                    } else {
                        printf("[ERROR] remove_from_queue returned NULL in MLFQ (Priority %d)\n", i + 1);
                    }
                    break;
                }
            }
            break;
    }
    if (next_process) {
        set_pcb_state(next_process, RUNNING);
        printf("[DEBUG] ▶️▶️ PID %d is now RUNNING (Priority: %d)\n", next_process->pid, next_process->priority);
    }
    return next_process;
}

// Update scheduler state
void update_scheduler(Scheduler* scheduler) {
    if (!scheduler) return;

    if (scheduler->running_process) {
        if (is_in_blocked_queue(scheduler, scheduler->running_process)) {
            printf("✅ PID %d is in the blocked queue.\n", scheduler->running_process->pid);
        } else {
            printf("❌ PID %d is NOT in the blocked queue.\n", scheduler->running_process->pid);
        }
    }

    // Check if the running process is still running
    if (scheduler->running_process) {
        if (scheduler->running_process->state == TERMINATED) {
            printf("✅ [INFO] Process PID=%d has finished execution at clock cycle %d.\n",
                scheduler->running_process->pid,
                scheduler->clock_cycle);
            // Remove the terminated process (if needed)
            scheduler->running_process = NULL;
        }
        else if (scheduler->algorithm != FCFS) {
            scheduler->running_process->quantum_remaining--;
            if (scheduler->algorithm == RR) {
                printf("[Round Robin] PID %d quantum left: %d\n",
                    scheduler->running_process->pid,
                    scheduler->running_process->quantum_remaining);
            }
            if (scheduler->running_process->quantum_remaining <= 0) {
                printf("⏳ [INFO] Quantum expired for PID %d, re-queuing.\n",
                    scheduler->running_process->pid);
                set_pcb_state(scheduler->running_process, READY);
                if (scheduler->algorithm == MLFQ) {
                    int current_priority = scheduler->running_process->priority;
                    if (current_priority < 4) {
                        set_pcb_priority(scheduler->running_process, current_priority + 1);
                        printf("🔄 [MLFQ] PID %d demoted to priority %d.\n",
                            scheduler->running_process->pid,
                            scheduler->running_process->priority);
                    }
                }
                add_process(scheduler, scheduler->running_process);
                scheduler->running_process = NULL;
            }
        }
    }

    // If no process is running, pick the next one
    if (!scheduler->running_process) {
        scheduler->running_process = schedule_next_process(scheduler);
        if (scheduler->running_process) {
            printf("▶️ [INFO] Scheduled PID %d to run (Priority: %d).\n",
                scheduler->running_process->pid,
                scheduler->running_process->priority);
    } else {
            printf("⚠️ [INFO] No process scheduled to run at clock cycle %d.\n",
                scheduler->clock_cycle);
        }
    }
}

void print_scheduler_status(const Scheduler* scheduler) {
    if (!scheduler) return;

    printf("\n==================== Scheduler Debug ====================\n");
    printf("Clock Cycle: %d\n", scheduler->clock_cycle);
    printf("Algorithm: %s\n", 
        scheduler->algorithm == FCFS ? "FCFS" : 
        scheduler->algorithm == RR ? "Round Robin" : "MLFQ");

    if (scheduler->running_process) {
        printf("Running Process: PID %d | Priority: %d | PC: %d\n",
            scheduler->running_process->pid,
            scheduler->running_process->priority,
            scheduler->running_process->program_counter);
    } else {
        printf("No process is currently running.\n");
    }

    printf("\nReady Queues:\n");
    for (int i = 0; i < 4; i++) {
        printf("  Priority %d (%d processes): ", i + 1, scheduler->ready_queues[i].size);
        for (int j = 0; j < scheduler->ready_queues[i].size; j++) {
            PCB* p = scheduler->ready_queues[i].processes[j];
            printf("[PID %d] ", p->pid);
        }
        printf("\n");
    }

    printf("\nBlocked Queue (%d processes): ", scheduler->blocked_queue.size);
    for (int j = 0; j < scheduler->blocked_queue.size; j++) {
        PCB* p = scheduler->blocked_queue.processes[j];
        printf("[PID %d] ", p->pid);
    }
    printf("\n========================================================\n\n");
}

void destroy_scheduler(Scheduler* scheduler) {
    print_queues_state(scheduler);
    printf("[TRACE] destroy_scheduler called! Cleaning up memory...\n");
    if (!scheduler) return;
    for (int i = 0; i < 4; i++) {
        if (scheduler->ready_queues[i].processes) {
            free(scheduler->ready_queues[i].processes);
            scheduler->ready_queues[i].processes = NULL;
            scheduler->ready_queues[i].size = 0;
            scheduler->ready_queues[i].capacity = 0;
        }
    }
    if (scheduler->blocked_queue.processes) {
        free(scheduler->blocked_queue.processes);
        scheduler->blocked_queue.processes = NULL;
    }
    print_queues_state(scheduler);

}


void scheduler_step() {
    print_scheduler_status(scheduler);
    scheduler->clock_cycle++;

    for (int i = 0; i < pending_list.count; ) {
        PCB* pcb = pending_list.list[i];
        if (pcb->program_counter >= pcb->instruction_count) {
            printf("[ERROR] PCB program_counter (%d) >= instruction_count (%d) for PID %d\n",
                pcb->program_counter, pcb->instruction_count, pcb->pid);
        }
        if (pcb->arrival_time <= scheduler->clock_cycle) {
            add_process(scheduler, pcb);
            for (int j = i; j < pending_list.count - 1; j++)
                pending_list.list[j] = pending_list.list[j + 1];
            pending_list.count--;
        } else {
            i++;
        }
    }

    PCB* pcb = scheduler->running_process;
    if (!pcb) {
        printf("[TRACE] No running process found, attempting to schedule...\n");
        pcb = schedule_next_process(scheduler);
        if (!pcb) {
            log_event(&logger, " No process to schedule.");
            return;
        }
    }

    printf("[DEBUG] >>> PCB before execution: PID=%d, PC=%d, State=%d\n",
        pcb->pid, pcb->program_counter, pcb->state);

    set_pcb_state(pcb, RUNNING);

    printf("[DEBUG] Executing instruction for PID=%d | PC=%d | InstructionCount=%d\n",
        pcb->pid, pcb->program_counter, pcb->instruction_count);

    bool success = false;
    PCB* unblocked_pcb = execute_instruction(pcb, &memory, &resource_manager, &logger, &success);
    if (unblocked_pcb) {
        printf("[DEBUG] 🔓🔓 PID %d is UNBLOCKED and re-added to READY queue\n", unblocked_pcb->pid);
        add_process(scheduler, unblocked_pcb);
        printf("[DEBUG] ✅✅✅ Unblocked PID %d and re-added to READY queue (Priority: %d)\n", unblocked_pcb->pid, unblocked_pcb->priority);
    }

    if (!success) {
        // The PCB state is set to BLOCKED inside sem_wait() in mutex.c, not here.
        printf("[DEBUG] 🚫🚫🚫 PID %d is BLOCKED after execution (Instruction: %s)\n", pcb->pid, pcb->instructions[pcb->program_counter - 1]);
        if (pcb->state == BLOCKED && !is_in_blocked_queue(scheduler, pcb)) {
            add_to_queue(&scheduler->blocked_queue, pcb);
            printf("[INFO] PID %d added to blocked queue after execution failure.\n", pcb->pid);
        }
        else if (pcb->state == BLOCKED) {
            printf("[DEBUG] PID %d is already in blocked queue, skipping add.\n", pcb->pid);
        }
        scheduler->running_process = NULL;
    } else if (pcb->program_counter >= pcb->instruction_count) {
        set_pcb_state(pcb, TERMINATED);
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), " [PID %d] Process completed.", pcb->pid);
        log_event(&logger, log_msg);
        scheduler->running_process = NULL;
    } else if (pcb->state != TERMINATED) {
        /* process has executed successfully and has more instructions */
        if (pcb->state == BLOCKED) {
            /* it blocked during the instruction */
            printf("[DEBUG] Skipping re‑adding PID %d because it is BLOCKED.\n", pcb->pid);
            scheduler->running_process = NULL;
        } else { /* still runnable */
            if (scheduler->algorithm == FCFS) {
                /* FCFS: keep the same process on the CPU */
                scheduler->running_process = pcb;       /* leave it running */
                /* state already RUNNING, nothing else to do */
            } else {
                /* RR / MLFQ: pre‑empt and re‑queue */
                set_pcb_state(pcb, READY);
                add_process(scheduler, pcb);
                scheduler->running_process = NULL;
            }
        }
    }

    if (scheduler->running_process == NULL &&
        is_all_queues_empty(scheduler) &&
        scheduler->blocked_queue.size == 0) {
        
        log_event(&logger, "✅✅ All processes have completed. System is idle.");
        printf("[DEBUG] Finished all processes. NOT resetting anything!\n");
    }
    else if (is_all_queues_empty(scheduler) && scheduler->blocked_queue.size > 0) {
        printf("⚠️ [WARN] All ready queues are empty but blocked queue has %d processes. Waiting for unblock.\n",
            scheduler->blocked_queue.size);
    }

    printf("[TRACE] ✅✅ END of scheduler_step (Clock: %d) | Ready: %d | Blocked: %d\n",
           scheduler->clock_cycle,
           !is_all_queues_empty(scheduler),
           scheduler->blocked_queue.size);
    print_scheduler_status(scheduler);
}

bool is_all_queues_empty(Scheduler* s) {
    for (int i = 0; i < 4; i++) {
        if (s->ready_queues[i].size > 0) return false;
    }
    return true;
}


void print_queues_state(Scheduler* scheduler) {
    printf("\n[TRACE] Queues pointer check:\n");
    for (int i = 0; i < 4; i++) {
        printf("  ready_queues[%d]: size=%d, capacity=%d, processes=%p\n",
            i,
            scheduler->ready_queues[i].size,
            scheduler->ready_queues[i].capacity,
            scheduler->ready_queues[i].processes);
    }
    printf("  blocked_queue: size=%d, capacity=%d, processes=%p\n",
        scheduler->blocked_queue.size,
        scheduler->blocked_queue.capacity,
        scheduler->blocked_queue.processes);
}

bool is_in_blocked_queue(Scheduler* scheduler, PCB* pcb) {
    for (int i = 0; i < scheduler->blocked_queue.size; i++) {
        if (scheduler->blocked_queue.processes[i] == pcb) {
            return true;
        }
    }
    return false;
}