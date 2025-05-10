#ifndef MUTEX_H
#define MUTEX_H

#include "pcb.h"
#include "logger.h"  

typedef enum {
    RESOURCE_USER_INPUT,
    RESOURCE_USER_OUTPUT,
    RESOURCE_FILE,
    NUM_RESOURCES
} ResourceType;

typedef struct {
    int locked;
    int owner_pid;
    int queue_size;
    int queue_capacity;
    PCB** waiting_queue;
} Mutex;

typedef struct {
    Mutex mutexes[NUM_RESOURCES];
} ResourceManager;

void init_resource_manager(ResourceManager* manager);
bool sem_wait(ResourceManager* manager, ResourceType resource, PCB* pcb, Logger* logger);
PCB* sem_signal(ResourceManager* manager, ResourceType resource, PCB* pcb, Logger* logger);
const char* get_resource_name(ResourceType resource);

#endif // MUTEX_H