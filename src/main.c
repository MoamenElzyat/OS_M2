#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/pcb.h"
#include "../include/memory.h"
#include "../include/mutex.h"
#include "../include/scheduler.h"
#include "../include/interpreter.h"

int main(int argc, char *argv[]) {
    Scheduler scheduler;
    Memory memory;
    ResourceManager resources;
    PCB* processes[3] = {NULL, NULL, NULL};

    SchedulingAlgorithm algorithm = FCFS;
    int quantum = 1;

    // Parse arguments
    if (argc >= 2) {
        if (strcmp(argv[1], "FCFS") == 0) algorithm = FCFS;
        else if (strcmp(argv[1], "RR") == 0) {
            algorithm = RR;
            if (argc > 2) {
                quantum = atoi(argv[2]);
                if (quantum <= 0) {
                    printf("Invalid quantum, using default 2\n");
                    quantum = 2;
                }
            }
        } else if (strcmp(argv[1], "MLFQ") == 0) algorithm = MLFQ;
        else {
            printf("Unknown algorithm, defaulting to FCFS\n");
        }
    }

    init_scheduler(&scheduler, algorithm, quantum);
    init_memory(&memory);
    init_resource_manager(&resources);

    const char* program_files[] = {
        "./program1.txt",
        "./program2.txt",
        "./program3.txt"
    };

    for (int i = 0; i < 3; i++) {
        processes[i] = create_pcb(i + 1, 0);
        if (!load_program(processes[i], program_files[i])) {
            printf("Failed to load %s\n", program_files[i]);
            return 1;
        }
        add_process(&scheduler, processes[i]);
    }

    printf("\n🚀 Starting manual execution with algorithm: %s\n\n",
           algorithm == FCFS ? "FCFS" :
           algorithm == RR ? "Round Robin" : "MLFQ");

    while (1) {
        PCB* pcb = schedule_next_process(&scheduler);
        if (!pcb) break;

        set_pcb_state(pcb, RUNNING);
        printf("▶️ Running process PID: %d\n", pcb->pid);

        bool finished = false;
        while (!finished) {
            bool success = false;
            PCB* unblocked_pcb = execute_instruction(pcb, &memory, &resources, &success);
            if (unblocked_pcb) {
                printf("🚀 Process PID %d unblocked and added back to scheduler.\n", unblocked_pcb->pid);
                add_process(&scheduler, unblocked_pcb);
            }

            // ✅ FCFS: process keeps running until all instructions finish.
            if (!success || pcb->program_counter >= pcb->instruction_count) {
                printf("✅ Process PID %d finished.\n", pcb->pid);
                set_pcb_state(pcb, TERMINATED);
                finished = true;
            }
        }
    }

    printf("\n✅ All processes completed.\n");

    destroy_scheduler(&scheduler);
    for (int i = 0; i < 3; i++) {
        if (processes[i]) destroy_pcb(processes[i]);
    }
    return 0;
}