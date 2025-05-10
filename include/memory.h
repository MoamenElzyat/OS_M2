#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include "pcb.h"

#define MEMORY_SIZE 60  

typedef struct {
    char* name;       
    char* data;       
    int process_id;  
} MemoryWord;

typedef struct {
    MemoryWord words[MEMORY_SIZE];  
    int next_free_word;            
} Memory;

void init_memory(Memory* memory);

int allocate_memory(Memory* memory, PCB* pcb, int size);

void deallocate_memory(Memory* memory, PCB* pcb);

void write_memory(Memory* memory, int address, const char* name, const char* data, int process_id);

void read_memory(const Memory* memory, int address, char** name, char** data, int* process_id);

void print_memory(const Memory* memory);

bool is_memory_available(const Memory* memory, int size);

#endif  // MEMORY_H