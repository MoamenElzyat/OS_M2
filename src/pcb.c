#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/pcb.h"
#include "globals.h"    
#include "interpreter.h"
#include "memory.h"
#include "mutex.h"
#include "queue.h" 

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

    // Confirmed all allocated memory is freed properly.

    free(pcb);
}

// Set state
void set_pcb_state(PCB* pcb, ProcessState state) {
    if (pcb) {
        printf("[DEBUG] set_pcb_state: PID=%d, Changing state from %s to %s\n",
               pcb->pid,
               get_state_string(pcb->state),
               get_state_string(state));
        pcb->state = state;
    } else {
        printf("[DEBUG] set_pcb_state: NULL pcb pointer received!\n");
    }
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
    if (!pcb->instructions[pcb->instruction_count]) {
        // handle strdup failure
        return;
    }
    pcb->instruction_count++;
}

// Update variable or create new if not exist
// Update variable or create new if not exist
void update_pcb_variable(PCB* pcb, const char* name, const char* value) {
    if (!pcb || !name || !value) return;

    for (int i = 0; i < pcb->var_count; i++) {
        if (strcmp(pcb->variables[i], name) == 0) {
            free(pcb->values[i]);
            pcb->values[i] = strdup(value);

            int mem_addr = pcb->memory_lower_bound + pcb->instruction_count + i;  
            write_memory(&memory, mem_addr, name, value, pcb->pid);
            return;
        }
    }

    char** new_vars = realloc(pcb->variables, (pcb->var_count + 1) * sizeof(char*));
    char** new_vals = realloc(pcb->values, (pcb->var_count + 1) * sizeof(char*));
    if (!new_vars || !new_vals) {
        // handle memory error safely
        return;
    }
    pcb->variables = new_vars;
    pcb->values = new_vals;

    pcb->variables[pcb->var_count] = strdup(name);
    pcb->values[pcb->var_count] = strdup(value);
    if (!pcb->variables[pcb->var_count] || !pcb->values[pcb->var_count]) {
        // handle strdup failure
        return;
    }

    int mem_addr = pcb->memory_lower_bound + pcb->instruction_count + pcb->var_count;
    write_memory(&memory, mem_addr, name, value, pcb->pid);

    pcb->var_count++;
}


void update_pcb_state_in_memory(PCB* pcb) {
    if (!pcb) return;

    char state_str[32];
    snprintf(state_str, sizeof(state_str), "State: %s", get_state_string(pcb->state));

    write_memory(&memory, pcb->memory_lower_bound, "ProcessState", state_str, pcb->pid);
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