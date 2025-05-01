#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/mutex.h"

#define INITIAL_QUEUE_CAPACITY 10

// Initialize resource manager
void init_resource_manager(ResourceManager* manager) {
    if (!manager) return;

    for (int i = 0; i < NUM_RESOURCES; i++) {
        manager->mutexes[i].locked = false;
        manager->mutexes[i].owner_pid = -1;
        manager->mutexes[i].queue_size = 0;
        manager->mutexes[i].queue_capacity = INITIAL_QUEUE_CAPACITY;
        manager->mutexes[i].waiting_queue = malloc(INITIAL_QUEUE_CAPACITY * sizeof(PCB*));
    }
}

// Semaphore wait operation
bool sem_wait(ResourceManager* manager, ResourceType resource, PCB* pcb) {
    if (!manager || !pcb || resource >= NUM_RESOURCES) return false;

    Mutex* mutex = &manager->mutexes[resource];

    if (!mutex->locked) {
        // Resource is available, acquire it
        mutex->locked = true;
        mutex->owner_pid = pcb->pid;
        return true;
    } else {
        // Resource is locked, add to waiting queue
        if (mutex->queue_size >= mutex->queue_capacity) {
            // Resize queue if needed
            mutex->queue_capacity *= 2;
            mutex->waiting_queue = realloc(mutex->waiting_queue, 
                                         mutex->queue_capacity * sizeof(PCB*));
        }

        // Add to queue based on priority
        int insert_pos = 0;
        for (; insert_pos < mutex->queue_size; insert_pos++) {
            if (pcb->priority < mutex->waiting_queue[insert_pos]->priority) {
                break;
            }
        }

        // Shift elements to make room
        for (int i = mutex->queue_size; i > insert_pos; i--) {
            mutex->waiting_queue[i] = mutex->waiting_queue[i-1];
        }

        mutex->waiting_queue[insert_pos] = pcb;
        mutex->queue_size++;
        set_pcb_state(pcb, BLOCKED);
        return false;
    }
}

// Semaphore signal operation
bool sem_signal(ResourceManager* manager, ResourceType resource, PCB* pcb) {
    if (!manager || !pcb || resource >= NUM_RESOURCES) return false;

    Mutex* mutex = &manager->mutexes[resource];

    if (mutex->locked && mutex->owner_pid == pcb->pid) {
        if (mutex->queue_size > 0) {
            // Give resource to highest priority waiting process
            PCB* next_pcb = mutex->waiting_queue[0];
            
            // Shift queue
            for (int i = 0; i < mutex->queue_size - 1; i++) {
                mutex->waiting_queue[i] = mutex->waiting_queue[i+1];
            }
            mutex->queue_size--;

            mutex->owner_pid = next_pcb->pid;
            set_pcb_state(next_pcb, READY);
        } else {
            // No processes waiting, release resource
            mutex->locked = false;
            mutex->owner_pid = -1;
        }
        return true;
    }
    return false;
}

// Print resource status
void print_resource_status(const ResourceManager* manager) {
    if (!manager) return;

    printf("Resource Status:\n");
    printf("Resource\tStatus\tOwner PID\tWaiting Processes\n");
    printf("------------------------------------------------\n");

    for (int i = 0; i < NUM_RESOURCES; i++) {
        printf("%s\t\t%s\t%d\t\t%d\n",
               get_resource_name(i),
               manager->mutexes[i].locked ? "Locked" : "Free",
               manager->mutexes[i].owner_pid,
               manager->mutexes[i].queue_size);
    }
}

// Get resource name
const char* get_resource_name(ResourceType resource) {
    switch (resource) {
        case RESOURCE_USER_INPUT: return "User Input";
        case RESOURCE_USER_OUTPUT: return "User Output";
        case RESOURCE_FILE: return "File";
        default: return "Unknown";
    }
} 