#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/pcb.h"

// Create a new PCB
PCB* create_pcb(int pid, int arrival_time) {
    PCB* pcb = (PCB*)malloc(sizeof(PCB));
    if (!pcb) return NULL;

    pcb->pid = pid;
    pcb->state = NEW;
    pcb->priority = 1;  // Default highest priority
    pcb->program_counter = 0;
    pcb->memory_lower_bound = -1;
    pcb->memory_upper_bound = -1;
    pcb->arrival_time = arrival_time;
    pcb->quantum_remaining = 0;
    pcb->time_in_queue = 0;
    pcb->var_count = 0;
    pcb->instruction_count = 0;
    pcb->variables = NULL;
    pcb->instructions = NULL;

    return pcb;
}

// Destroy a PCB and free its memory
void destroy_pcb(PCB* pcb) {
    if (!pcb) return;

    // Free variables
    for (int i = 0; i < pcb->var_count; i++) {
        free(pcb->variables[i]);
    }
    free(pcb->variables);

    // Free instructions
    for (int i = 0; i < pcb->instruction_count; i++) {
        free(pcb->instructions[i]);
    }
    free(pcb->instructions);

    free(pcb);
}

// Set PCB state
void set_pcb_state(PCB* pcb, ProcessState state) {
    if (pcb) pcb->state = state;
}

// Set PCB priority
void set_pcb_priority(PCB* pcb, int priority) {
    if (pcb && priority >= 1 && priority <= 4) {
        pcb->priority = priority;
    }
}

// Set PCB memory bounds
void set_pcb_memory_bounds(PCB* pcb, int lower, int upper) {
    if (pcb && lower >= 0 && upper >= lower) {
        pcb->memory_lower_bound = lower;
        pcb->memory_upper_bound = upper;
    }
}

// Add a variable to PCB
void add_pcb_variable(PCB* pcb, const char* name, const char* value) {
    if (!pcb || !name || !value) return;

    // Allocate space for new variable
    char** new_vars = realloc(pcb->variables, (pcb->var_count + 1) * sizeof(char*));
    if (!new_vars) return;

    pcb->variables = new_vars;
    pcb->variables[pcb->var_count] = strdup(name);
    pcb->var_count++;
}

// Add an instruction to PCB
void add_pcb_instruction(PCB* pcb, const char* instruction) {
    if (!pcb || !instruction) return;

    // Allocate space for new instruction
    char** new_instructions = realloc(pcb->instructions, (pcb->instruction_count + 1) * sizeof(char*));
    if (!new_instructions) return;

    pcb->instructions = new_instructions;
    pcb->instructions[pcb->instruction_count] = strdup(instruction);
    pcb->instruction_count++;
}

// Get string representation of process state
const char* get_state_string(ProcessState state) {
    switch (state) {
        case NEW: return "NEW";
        case READY: return "READY";
        case RUNNING: return "RUNNING";
        case BLOCKED: return "BLOCKED";
        case TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
} 