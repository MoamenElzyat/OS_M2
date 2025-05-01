#ifndef MUTEX_H
#define MUTEX_H

#include "pcb.h"

// Resource types
typedef enum {
    RESOURCE_USER_INPUT,
    RESOURCE_USER_OUTPUT,
    RESOURCE_FILE,
    NUM_RESOURCES
} ResourceType;

// Mutex structure
typedef struct {
    bool locked;
    int owner_pid;
    PCB** waiting_queue;
    int queue_size;
    int queue_capacity;
} Mutex;

// Resource manager structure
typedef struct {
    Mutex mutexes[NUM_RESOURCES];
} ResourceManager;

// Function declarations
void init_resource_manager(ResourceManager* manager);
bool sem_wait(ResourceManager* manager, ResourceType resource, PCB* pcb);
bool sem_signal(ResourceManager* manager, ResourceType resource, PCB* pcb);
void print_resource_status(const ResourceManager* manager);
const char* get_resource_name(ResourceType resource);

#endif // MUTEX_H 