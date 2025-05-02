#include "mutex.h"
#include <stdio.h>
#include <stdlib.h>

#define INITIAL_QUEUE_CAPACITY 10

void init_resource_manager(ResourceManager* manager) {
    if (!manager) return;
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

    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg),
             "[PID %d] Requests resource [%s]", pcb->pid, get_resource_name(resource));
    log_event(logger, log_msg);

    if (!mutex->locked || mutex->owner_pid == pcb->pid) {
        mutex->locked = 1;
        mutex->owner_pid = pcb->pid;
        snprintf(log_msg, sizeof(log_msg),
                 "[PID %d] Acquired [%s]", pcb->pid, get_resource_name(resource));
        log_event(logger, log_msg);
        return true;
    } else {
        if (mutex->queue_size >= mutex->queue_capacity) {
            mutex->queue_capacity *= 2;
            mutex->waiting_queue = realloc(mutex->waiting_queue,
                                           mutex->queue_capacity * sizeof(PCB*));
        }

        int insert_pos = 0;
        for (; insert_pos < mutex->queue_size; insert_pos++) {
            if (pcb->priority < mutex->waiting_queue[insert_pos]->priority) break;
        }
        for (int i = mutex->queue_size; i > insert_pos; i--) {
            mutex->waiting_queue[i] = mutex->waiting_queue[i-1];
        }
        mutex->waiting_queue[insert_pos] = pcb;
        mutex->queue_size++;
        set_pcb_state(pcb, BLOCKED);
        snprintf(log_msg, sizeof(log_msg),
                 "[PID %d] BLOCKED on [%s] (queue size: %d)", pcb->pid,
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

        if (mutex->queue_size > 0) {
            unblocked_pcb = mutex->waiting_queue[0];
            for (int i = 0; i < mutex->queue_size - 1; i++) {
                mutex->waiting_queue[i] = mutex->waiting_queue[i + 1];
            }
            mutex->queue_size--;
            mutex->owner_pid = unblocked_pcb->pid;
            set_pcb_state(unblocked_pcb, READY);

            snprintf(log_msg, sizeof(log_msg),
                     "[PID %d] RELEASED [%s], [PID %d] UNBLOCKED and acquired",
                     pcb->pid, get_resource_name(resource), unblocked_pcb->pid);
            log_event(logger, log_msg);
        } else {
            mutex->locked = 0;
            mutex->owner_pid = -1;
            snprintf(log_msg, sizeof(log_msg),
                     "[PID %d] RELEASED [%s] (no waiting)", pcb->pid, get_resource_name(resource));
            log_event(logger, log_msg);
        }
        return unblocked_pcb;
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