#include "mutex.h"
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "interpreter.h"
#include "memory.h"
#include "pcb.h"
#include "queue.h"
#include "scheduler.h"

extern Scheduler* scheduler;

#define INITIAL_QUEUE_CAPACITY 10

void init_resource_manager(ResourceManager* manager) {
    if (!manager) return;
    printf("[DEBUG] Resource manager initialized.\n");
    for (int i = 0; i < NUM_RESOURCES; i++) {
        manager->mutexes[i].locked = 0;
        manager->mutexes[i].owner_pid = -1;
        manager->mutexes[i].queue_size = 0;
        manager->mutexes[i].queue_capacity = INITIAL_QUEUE_CAPACITY;
        manager->mutexes[i].waiting_queue = malloc(INITIAL_QUEUE_CAPACITY * sizeof(PCB*));
    }
}

bool sem_wait(ResourceManager* manager, ResourceType resource, PCB* pcb, Logger* logger) {
    if (!manager || !pcb || resource >= NUM_RESOURCES) return false;
    Mutex* mutex = &manager->mutexes[resource];
    printf("[DEBUG] sem_wait called: PID=%d, Resource=%s, Locked=%d, Owner=%d\n",
        pcb->pid, get_resource_name(resource), mutex->locked, mutex->owner_pid);

    char log_msg[256];

    if (!mutex->locked || mutex->owner_pid == pcb->pid) {
        mutex->locked = 1;
        mutex->owner_pid = pcb->pid;
        printf("[DEBUG] --> pcb->pid = %d\n", pcb->pid);
        snprintf(log_msg, sizeof(log_msg),
            "[Event] [Program: %s | PID %d] Acquired [%s]",
            pcb->program_name, pcb->pid, get_resource_name(resource));
        log_event(logger, log_msg);
        printf("[DEBUG] PID=%d acquired mutex on resource %s without blocking; no re-add to ready queue done.\n",
            pcb->pid, get_resource_name(resource));
        return true;
    } else {
        if (mutex->queue_size >= mutex->queue_capacity) {
            mutex->queue_capacity *= 2;
            PCB** new_queue = realloc(mutex->waiting_queue,
                                      mutex->queue_capacity * sizeof(PCB*));
            if (!new_queue) {
                fprintf(stderr, "Failed to realloc waiting_queue!\n");
                exit(EXIT_FAILURE);
            }
            mutex->waiting_queue = new_queue;
        }

        int insert_pos = 0;
        for (; insert_pos < mutex->queue_size; insert_pos++) {
            if (pcb->priority < mutex->waiting_queue[insert_pos]->priority) break;
        }

        // ✅ Check if it's already in the waiting queue
        bool already_waiting = false;
        for (int i = 0; i < mutex->queue_size; i++) {
            if (mutex->waiting_queue[i] == pcb) {
                already_waiting = true;
                break;
            }
        }

        if (!already_waiting) {
            for (int i = mutex->queue_size; i > insert_pos; i--) {
                mutex->waiting_queue[i] = mutex->waiting_queue[i - 1];
            }
            mutex->waiting_queue[insert_pos] = pcb;
            mutex->queue_size++;
            set_pcb_state(pcb, BLOCKED);
            printf("[DEBUG] set_pcb_state called for PID=%d | priority=%d | program_name=%s\n",
                   pcb->pid, pcb->priority, pcb->program_name);
            printf("[DEBUG] Set PID=%d state to BLOCKED after being queued on resource %s\n", pcb->pid, get_resource_name(resource));
        } else {
            printf("[DEBUG] PID=%d is already in waiting queue for resource %s; skipping insert.\n",
                   pcb->pid, get_resource_name(resource));
        }

        printf("[DEBUG] Mutex queue status after insert: queue_size=%d, queue_capacity=%d, mutex=%p, waiting_queue ptr=%p\n",
               mutex->queue_size, mutex->queue_capacity, (void*)mutex, (void*)mutex->waiting_queue);
        for (int dbg_i = 0; dbg_i < mutex->queue_size; dbg_i++) {
            printf("[DEBUG] waiting_queue[%d] = PID %d (ptr: %p)\n", dbg_i, mutex->waiting_queue[dbg_i]->pid, (void*)mutex->waiting_queue[dbg_i]);
        }
        snprintf(log_msg, sizeof(log_msg),
            "[Event] [Program: %s | PID %d] Blocked on [%s] (queue size: %d)",
            pcb->program_name, pcb->pid,
            get_resource_name(resource), mutex->queue_size);
        log_event(logger, log_msg);
        return false;
    }
}

PCB* sem_signal(ResourceManager* manager, ResourceType resource, PCB* pcb, Logger* logger) {
    if (!manager || !pcb || resource >= NUM_RESOURCES) return NULL;
    Mutex* mutex = &manager->mutexes[resource];

    if (mutex->locked && mutex->owner_pid == pcb->pid) {
        PCB* unblocked_pcb = NULL;
        char log_msg[256];

        snprintf(log_msg, sizeof(log_msg),
            "[Event] [Program: %s | PID %d] Released [%s]",
            pcb->program_name, pcb->pid, get_resource_name(resource));
        log_event(logger, log_msg);

        if (mutex->queue_size > 0) {
            unblocked_pcb = mutex->waiting_queue[0];
            for (int i = 0; i < mutex->queue_size - 1; i++) {
                mutex->waiting_queue[i] = mutex->waiting_queue[i + 1];
            }
            mutex->queue_size--;
            mutex->owner_pid = unblocked_pcb->pid;
            set_pcb_state(unblocked_pcb, READY);

            if (scheduler != NULL) {
                // ✅ Remove from blocked queue if present
                for (int i = 0; i < scheduler->blocked_queue.size; i++) {
                    if (scheduler->blocked_queue.processes[i] == unblocked_pcb) {
                        remove_from_queue(&scheduler->blocked_queue, i);
                        printf("[DEBUG] Removed PID=%d from blocked queue after unblocking.\n", unblocked_pcb->pid);
                        break;
                    }
                }

                // ✅ Check if it's already in ready queue to prevent duplication
                bool already_ready = false;
                ProcessQueue* queue = &scheduler->ready_queues[unblocked_pcb->priority - 1];
                for (int j = 0; j < queue->size; j++) {
                    if (queue->processes[j] == unblocked_pcb) {
                        already_ready = true;
                        break;
                    }
                }

                if (!already_ready) {
                    add_process(scheduler, unblocked_pcb);
                    printf("[DEBUG] Re-added PID=%d to ready queue after unblocking.\n", unblocked_pcb->pid);
                } else {
                    printf("[DEBUG] Unblocked PID=%d was already in ready queue; skipping add.\n", unblocked_pcb->pid);
                }
            }

            snprintf(log_msg, sizeof(log_msg),
                "[Event] [Program: %s | PID %d] Unblocked from [%s]",
                unblocked_pcb->program_name, unblocked_pcb->pid, get_resource_name(resource));
            log_event(logger, log_msg);

            snprintf(log_msg, sizeof(log_msg),
                "[Event] [Program: %s | PID %d] Acquired [%s]",
                unblocked_pcb->program_name, unblocked_pcb->pid, get_resource_name(resource));
            log_event(logger, log_msg);

            printf("[DEBUG] sem_signal unblocked PID=%d on resource %s\n",
                unblocked_pcb ? unblocked_pcb->pid : -1,
                get_resource_name(resource));
        } else {
            mutex->locked = 0;
            mutex->owner_pid = -1;
        }
        return unblocked_pcb;
    } else {
        printf("[DEBUG] sem_signal: No action taken (PID=%d does not own %s or resource is unlocked)\n",
            pcb->pid, get_resource_name(resource));
    }
    return NULL;
}
const char* get_resource_name(ResourceType resource) {
    switch (resource) {
        case RESOURCE_USER_INPUT: return "User Input";
        case RESOURCE_USER_OUTPUT: return "User Output";
        case RESOURCE_FILE: return "File";
        default: return "Unknown";
    }
}