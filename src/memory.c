#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/memory.h"
#include "globals.h"
#include "interpreter.h"
#include "pcb.h"
#include "queue.h"

#include <assert.h>


void init_memory(Memory* memory) {
    if (!memory) return;
    printf("[DEBUG] Initializing memory...\n");
    for (int i = 0; i < MEMORY_SIZE; i++) {
        memory->words[i].name = NULL;
        memory->words[i].data = NULL;
        memory->words[i].process_id = 0;
    }
    memory->next_free_word = 0;
    printf("[DEBUG] Memory initialized.\n");
}

int allocate_memory(Memory* memory, PCB* pcb, int size) {
    printf("[DEBUG] Request to allocate %d units for PID %d.\n", size, pcb->pid);
    if (!memory || !pcb || size <= 0 || size > MEMORY_SIZE) {
        printf("[ERROR] Invalid arguments or size too large.\n");
        return -1;
    }

    int start = -1, count = 0;
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (memory->words[i].process_id == 0) {
            if (count == 0) start = i;
            count++;
            if (count == size) break;
        } else {
            count = 0;
            start = -1;
        }
    }

    if (count < size) {
        printf("[ERROR] Not enough contiguous memory for PID %d.\n", pcb->pid);
        return -1;
    }

    for (int i = start; i < start + size; i++) {
        memory->words[i].process_id = pcb->pid;
    }
    set_pcb_memory_bounds(pcb, start, start + size - 1);
    printf("[DEBUG] Allocated memory for PID %d from %d to %d.\n", pcb->pid, start, start + size - 1);
    return start;
}

void deallocate_memory(Memory* memory, PCB* pcb) {
    printf("[DEBUG] Deallocating memory for PID %d...\n", pcb->pid);
    if (!memory || !pcb) return;
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (memory->words[i].process_id == pcb->pid) {
            free(memory->words[i].name);
            free(memory->words[i].data);
            memory->words[i].name = NULL;
            memory->words[i].data = NULL;
            memory->words[i].process_id = 0;
        }
    }
    printf("[DEBUG] Deallocated memory for PID %d.\n", pcb->pid);
}


void write_memory(Memory* memory, int address, const char* name, const char* data, int process_id) {
    printf("[DEBUG] Writing to memory at address %d (PID %d)...\n", address, process_id);
    assert(memory != NULL);
    assert(address >= 0 && address < MEMORY_SIZE);

    free(memory->words[address].name);
    free(memory->words[address].data);

    memory->words[address].name = name ? strdup(name) : NULL;
    if (name && !memory->words[address].name) {
        printf("[ERROR] Failed to allocate memory for name at address %d.\n", address);
    }
    memory->words[address].data = data ? strdup(data) : NULL;
    if (data && !memory->words[address].data) {
        printf("[ERROR] Failed to allocate memory for data at address %d.\n", address);
    }
    memory->words[address].process_id = process_id;

    printf("[DEBUG] Wrote: name='%s', data='%s' at %d.\n", name, data, address);
}

void read_memory(const Memory* memory, int address, char** name, char** data, int* process_id) {
    if (!memory || address < 0 || address >= MEMORY_SIZE) {
        printf("[ERROR] Invalid memory read at %d\n", address);
        return;
    }

    if (name) {
        if (memory->words[address].name) {
            *name = memory->words[address].name;
        } else {
            *name = NULL;
        }
    }
    if (data) {
        if (memory->words[address].data) {
            *data = memory->words[address].data;
        } else {
            *data = NULL;
        }
    }
    if (process_id) {
        *process_id = memory->words[address].process_id;
    }
}

void print_memory(const Memory* memory) {
    if (!memory) return;
    printf("=========== Memory Contents ===========\n");
    printf("Addr\tName\tData\tPID\n");
    printf("---------------------------------------\n");
    for (int i = 0; i < MEMORY_SIZE; i++) {
        printf("%d\t%s\t%s\t%d\n",
            i,
            memory->words[i].name ? memory->words[i].name : "-",
            memory->words[i].data ? memory->words[i].data : "-",
            memory->words[i].process_id);
    }
    printf("=======================================\n");
}

bool is_memory_available(const Memory* memory, int size) {
    if (!memory || size <= 0 || size > MEMORY_SIZE) return false;
    int count = 0;
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (memory->words[i].process_id == 0) {
            count++;
            if (count == size) return true;
        } else {
            count = 0;
        }
    }
    return false;
}
