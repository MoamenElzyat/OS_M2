#include <stdio.h>
#include <assert.h>
#include "../include/scheduler.h"
#include "../include/pcb.h"

// Test FCFS scheduling
void test_fcfs_scheduler() {
    Scheduler scheduler;
    init_scheduler(&scheduler, FCFS, 1);

    PCB* pcb1 = create_pcb(1, 0);
    PCB* pcb2 = create_pcb(2, 0);
    PCB* pcb3 = create_pcb(3, 0);

    add_process(&scheduler, pcb1);
    add_process(&scheduler, pcb2);
    add_process(&scheduler, pcb3);

    PCB* next = schedule_next_process(&scheduler);
    assert(next->pid == 1);

    next = schedule_next_process(&scheduler);
    assert(next->pid == 2);

    next = schedule_next_process(&scheduler);
    assert(next->pid == 3);

    destroy_scheduler(&scheduler);
    destroy_pcb(pcb1);
    destroy_pcb(pcb2);
    destroy_pcb(pcb3);

    printf("test_fcfs_scheduler passed.\n");
}

// Test Round Robin scheduling
void test_rr_scheduler() {
    Scheduler scheduler;
    init_scheduler(&scheduler, RR, 2);  // quantum = 2

    PCB* pcb1 = create_pcb(1, 0);
    PCB* pcb2 = create_pcb(2, 0);

    add_process(&scheduler, pcb1);
    add_process(&scheduler, pcb2);

    PCB* next = schedule_next_process(&scheduler);
    assert(next->pid == 1);
    assert(next->quantum_remaining == 2);

    next = schedule_next_process(&scheduler);
    assert(next->pid == 2);
    assert(next->quantum_remaining == 2);

    destroy_scheduler(&scheduler);
    destroy_pcb(pcb1);
    destroy_pcb(pcb2);

    printf("test_rr_scheduler passed.\n");
}

// Test MLFQ scheduling
void test_mlfq_scheduler() {
    Scheduler scheduler;
    init_scheduler(&scheduler, MLFQ, 1);

    PCB* pcb1 = create_pcb(1, 0);
    PCB* pcb2 = create_pcb(2, 0);
    PCB* pcb3 = create_pcb(3, 0);

    set_pcb_priority(pcb1, 1);
    set_pcb_priority(pcb2, 2);
    set_pcb_priority(pcb3, 3);

    add_process(&scheduler, pcb1);
    add_process(&scheduler, pcb2);
    add_process(&scheduler, pcb3);

    PCB* next = schedule_next_process(&scheduler);
    assert(next->pid == 1);
    assert(next->quantum_remaining == 1);  // priority 1 -> quantum 1

    next = schedule_next_process(&scheduler);
    assert(next->pid == 2);
    assert(next->quantum_remaining == 2);  // priority 2 -> quantum 2

    next = schedule_next_process(&scheduler);
    assert(next->pid == 3);
    assert(next->quantum_remaining == 4);  // priority 3 -> quantum 4

    destroy_scheduler(&scheduler);
    destroy_pcb(pcb1);
    destroy_pcb(pcb2);
    destroy_pcb(pcb3);

    printf("test_mlfq_scheduler passed.\n");
}

int main() {
    test_fcfs_scheduler();
    test_rr_scheduler();
    test_mlfq_scheduler();
    printf("All scheduler tests passed.\n");
    return 0;
}