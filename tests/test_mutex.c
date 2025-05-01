// tests/test_mutex.c
#include <stdio.h>
#include <assert.h>
#include "../include/mutex.h"
#include "../include/pcb.h"

void test_resource_locking() {
    ResourceManager manager;
    init_resource_manager(&manager);

    PCB pcb1 = { .pid = 1, .priority = 1, .state = READY };
    PCB pcb2 = { .pid = 2, .priority = 2, .state = READY };

    // First sem_wait should acquire resource
    assert(sem_wait(&manager, RESOURCE_USER_OUTPUT, &pcb1) == true);
    assert(manager.mutexes[RESOURCE_USER_OUTPUT].locked == true);
    assert(manager.mutexes[RESOURCE_USER_OUTPUT].owner_pid == pcb1.pid);

    // Second sem_wait should block pcb2
    assert(sem_wait(&manager, RESOURCE_USER_OUTPUT, &pcb2) == false);
    assert(manager.mutexes[RESOURCE_USER_OUTPUT].queue_size == 1);
    assert(manager.mutexes[RESOURCE_USER_OUTPUT].waiting_queue[0]->pid == pcb2.pid);
    assert(pcb2.state == BLOCKED);

    // Now signal and check that pcb2 acquires the resource
    assert(sem_signal(&manager, RESOURCE_USER_OUTPUT, &pcb1) == true);
    assert(manager.mutexes[RESOURCE_USER_OUTPUT].owner_pid == pcb2.pid);
    assert(manager.mutexes[RESOURCE_USER_OUTPUT].queue_size == 0);
    assert(pcb2.state == READY);

    printf("test_resource_locking passed.\n");
}

void test_priority_queueing() {
    ResourceManager manager;
    init_resource_manager(&manager);

    PCB pcb1 = { .pid = 1, .priority = 3, .state = READY };
    PCB pcb2 = { .pid = 2, .priority = 1, .state = READY };
    PCB pcb3 = { .pid = 3, .priority = 2, .state = READY };

    assert(sem_wait(&manager, RESOURCE_FILE, &pcb1) == true);
    assert(sem_wait(&manager, RESOURCE_FILE, &pcb2) == false);
    assert(sem_wait(&manager, RESOURCE_FILE, &pcb3) == false);

    // Queue should be ordered by priority: pcb2, pcb3
    assert(manager.mutexes[RESOURCE_FILE].waiting_queue[0]->pid == 2);
    assert(manager.mutexes[RESOURCE_FILE].waiting_queue[1]->pid == 3);

    sem_signal(&manager, RESOURCE_FILE, &pcb1);
    assert(manager.mutexes[RESOURCE_FILE].owner_pid == pcb2.pid);

    sem_signal(&manager, RESOURCE_FILE, &pcb2);
    assert(manager.mutexes[RESOURCE_FILE].owner_pid == pcb3.pid);

    printf("test_priority_queueing passed.\n");
}
void test_mutex_blocking_many() {
    ResourceManager resources;
    init_resource_manager(&resources);

    const int max_procs = 20;
    PCB* pcbs[max_procs];
    for (int i = 0; i < max_procs; i++) {
        pcbs[i] = create_pcb(i + 1, 0);
        sem_wait(&resources, RESOURCE_FILE, pcbs[i]);  // كله يدخل البلوك
    }

    assert(resources.mutexes[RESOURCE_FILE].queue_size == max_procs - 1);
    printf("test_mutex_blocking_many passed.\n");

    // Clean up
    for (int i = 0; i < max_procs; i++) {
        destroy_pcb(pcbs[i]);
    }
}

int main() {
    test_resource_locking();
    test_priority_queueing();
    printf("All mutex tests passed.\n");
    return 0;
}
