#ifndef MEMORY_H
#define MEMORY_H

#include "pcb.h"

#define MEMORY_SIZE 60
#define MAX_VARIABLES_PER_PROCESS 3

// Memory word structure
typedef struct {
    char* name;
    char* data;
    int process_id;  // -1 if not allocated to any process
} MemoryWord;

// Memory management structure
typedef struct {
    MemoryWord words[MEMORY_SIZE];
    int next_free_word;
} Memory;

// Function declarations
void init_memory(Memory* memory);
int allocate_memory(Memory* memory, PCB* pcb, int size);
void deallocate_memory(Memory* memory, PCB* pcb);
void write_memory(Memory* memory, int address, const char* name, const char* data, int process_id);
void read_memory(const Memory* memory, int address, char** name, char** data, int* process_id);
void print_memory(const Memory* memory);
bool is_memory_available(const Memory* memory, int size);

#endif // MEMORY_H 