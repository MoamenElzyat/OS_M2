#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/memory.h"

// Initialize memory
void init_memory(Memory* memory) {
    if (!memory) return;

    for (int i = 0; i < MEMORY_SIZE; i++) {
        memory->words[i].name = NULL;
        memory->words[i].data = NULL;
        memory->words[i].process_id = -1;
    }
    memory->next_free_word = 0;
}

// Allocate memory for a process
int allocate_memory(Memory* memory, PCB* pcb, int size) {
    if (!memory || !pcb || size <= 0 || size > MEMORY_SIZE) return -1;

    // Check if there's enough contiguous free memory
    int start = -1;
    int count = 0;
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (memory->words[i].process_id == -1) {
            if (count == 0) start = i;
            count++;
            if (count == size) break;
        } else {
            count = 0;
            start = -1;
        }
    }

    if (count < size) return -1;  // Not enough contiguous memory

    // Allocate the memory
    for (int i = start; i < start + size; i++) {
        memory->words[i].process_id = pcb->pid;
    }

    // Update PCB memory bounds
    set_pcb_memory_bounds(pcb, start, start + size - 1);

    return start;
}

// Deallocate memory for a process
void deallocate_memory(Memory* memory, PCB* pcb) {
    if (!memory || !pcb) return;

    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (memory->words[i].process_id == pcb->pid) {
            free(memory->words[i].name);
            free(memory->words[i].data);
            memory->words[i].name = NULL;
            memory->words[i].data = NULL;
            memory->words[i].process_id = -1;
        }
    }
}

// Write to memory
void write_memory(Memory* memory, int address, const char* name, const char* data, int process_id) {
    if (!memory || address < 0 || address >= MEMORY_SIZE) return;

    // Free existing data if any
    free(memory->words[address].name);
    free(memory->words[address].data);

    // Allocate and copy new data
    memory->words[address].name = name ? strdup(name) : NULL;
    memory->words[address].data = data ? strdup(data) : NULL;
    memory->words[address].process_id = process_id;
}

// Read from memory
void read_memory(const Memory* memory, int address, char** name, char** data, int* process_id) {
    if (!memory || address < 0 || address >= MEMORY_SIZE) return;

    if (name) *name = memory->words[address].name ? strdup(memory->words[address].name) : NULL;
    if (data) *data = memory->words[address].data ? strdup(memory->words[address].data) : NULL;
    if (process_id) *process_id = memory->words[address].process_id;
}

// Print memory contents
void print_memory(const Memory* memory) {
    if (!memory) return;

    printf("Memory Contents:\n");
    printf("Address\tName\tData\tProcess ID\n");
    printf("--------------------------------\n");

    for (int i = 0; i < MEMORY_SIZE; i++) {
        printf("%d\t%s\t%s\t%d\n",
               i,
               memory->words[i].name ? memory->words[i].name : "-",
               memory->words[i].data ? memory->words[i].data : "-",
               memory->words[i].process_id);
    }
}

// Check if memory is available
bool is_memory_available(const Memory* memory, int size) {
    if (!memory || size <= 0 || size > MEMORY_SIZE) return false;

    int count = 0;
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (memory->words[i].process_id == -1) {
            count++;
            if (count == size) return true;
        } else {
            count = 0;
        }
    }

    return false;
} 