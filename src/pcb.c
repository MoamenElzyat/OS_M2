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
    pcb->priority = 1;
    pcb->program_counter = 0;
    pcb->memory_lower_bound = -1;
    pcb->memory_upper_bound = -1;
    pcb->arrival_time = arrival_time;
    pcb->quantum_remaining = 0;
    pcb->time_in_queue = 0;
    pcb->var_count = 0;
    pcb->variables = NULL;
    pcb->values = NULL;
    pcb->instruction_count = 0;
    pcb->instructions = NULL;

    return pcb;
}

// Destroy PCB
void destroy_pcb(PCB* pcb) {
    if (!pcb) return;

    for (int i = 0; i < pcb->var_count; i++) {
        free(pcb->variables[i]);
        free(pcb->values[i]);
    }
    free(pcb->variables);
    free(pcb->values);

    for (int i = 0; i < pcb->instruction_count; i++) {
        free(pcb->instructions[i]);
    }
    free(pcb->instructions);

    free(pcb);
}

// Set state
void set_pcb_state(PCB* pcb, ProcessState state) {
    if (pcb) pcb->state = state;
}

// Set priority
void set_pcb_priority(PCB* pcb, int priority) {
    if (pcb && priority >= 1 && priority <= 4) {
        pcb->priority = priority;
    }
}

// Set memory bounds
void set_pcb_memory_bounds(PCB* pcb, int lower, int upper) {
    if (pcb && lower >= 0 && upper >= lower) {
        pcb->memory_lower_bound = lower;
        pcb->memory_upper_bound = upper;
    }
}

// Add instruction
void add_pcb_instruction(PCB* pcb, const char* instruction) {
    if (!pcb || !instruction) return;

    char** new_instructions = realloc(pcb->instructions, (pcb->instruction_count + 1) * sizeof(char*));
    if (!new_instructions) return;

    pcb->instructions = new_instructions;
    pcb->instructions[pcb->instruction_count] = strdup(instruction);
    pcb->instruction_count++;
}

// Update variable or create new if not exist
void update_pcb_variable(PCB* pcb, const char* name, const char* value) {
    for (int i = 0; i < pcb->var_count; i++) {
        if (strcmp(pcb->variables[i], name) == 0) {
            free(pcb->values[i]);
            pcb->values[i] = strdup(value);
            return;
        }
    }
    pcb->variables = realloc(pcb->variables, (pcb->var_count + 1) * sizeof(char*));
    pcb->values = realloc(pcb->values, (pcb->var_count + 1) * sizeof(char*));
    pcb->variables[pcb->var_count] = strdup(name);
    pcb->values[pcb->var_count] = strdup(value);
    pcb->var_count++;
}

// Get variable value
const char* get_pcb_variable(PCB* pcb, const char* name) {
    for (int i = 0; i < pcb->var_count; i++) {
        if (strcmp(pcb->variables[i], name) == 0) {
            return pcb->values[i];
        }
    }
    return NULL;
}

// State string
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