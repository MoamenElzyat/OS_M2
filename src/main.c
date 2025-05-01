#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "../include/pcb.h"
#include "../include/memory.h"
#include "../include/mutex.h"
#include "../include/scheduler.h"
#include "../include/interpreter.h"
#include "../include/gui.h"

int main(int argc, char *argv[]) {
    // Initialize components
    Scheduler scheduler;
    Memory memory;
    ResourceManager resources;
    PCB* processes[3] = {NULL, NULL, NULL};

    // Default to FCFS
    SchedulingAlgorithm algorithm = FCFS;
    int quantum = 1;

    // Parse command line arguments if provided
    if (argc >= 2) {
        if (strcmp(argv[1], "FCFS") == 0) algorithm = FCFS;
        else if (strcmp(argv[1], "RR") == 0) {
            algorithm = RR;
            if (argc > 2) {
                quantum = atoi(argv[2]);
                if (quantum <= 0) {
                    printf("Invalid quantum value, using default of 2\n");
                    quantum = 2;
                }
            }
        }
        else if (strcmp(argv[1], "MLFQ") == 0) algorithm = MLFQ;
        else {
            printf("Invalid scheduling algorithm, defaulting to FCFS\n");
        }
    }

    init_scheduler(&scheduler, algorithm, quantum);
    init_memory(&memory);
    init_resource_manager(&resources);

    // Load programs
    const char* program_files[] = {
        "./program1.txt",
        "./program2.txt",
        "./program3.txt"
    };

    for (int i = 0; i < 3; i++) {
        processes[i] = create_pcb(i + 1, 0);
        if (!load_program(processes[i], program_files[i])) {
            printf("Failed to load program %s\n", program_files[i]);
            return 1;
        }
        add_process(&scheduler, processes[i]);
    }

    // Initialize and create GUI
    init_gui(&argc, &argv);
    create_gui(&scheduler, &memory, &resources);
    update_gui();
    show_gui();

    // Cleanup
    destroy_scheduler(&scheduler);
    for (int i = 0; i < 3; i++) {
        if (processes[i]) destroy_pcb(processes[i]);
    }

    return 0;
} 