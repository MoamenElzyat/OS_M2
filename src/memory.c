#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/memory.h"

// ✅ تهيئة الذاكرة بالكامل
void init_memory(Memory* memory) {
    if (!memory) return;
    for (int i = 0; i < MEMORY_SIZE; i++) {
        memory->words[i].name = NULL;
        memory->words[i].data = NULL;
        memory->words[i].process_id = -1;
    }
    memory->next_free_word = 0;
}

// ✅ تخصيص مساحة (بيدور على مساحة متتالية)
int allocate_memory(Memory* memory, PCB* pcb, int size) {
    if (!memory || !pcb || size <= 0 || size > MEMORY_SIZE) return -1;

    int start = -1, count = 0;
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

    if (count < size) return -1;  // مفيش مساحة كفاية

    for (int i = start; i < start + size; i++) {
        memory->words[i].process_id = pcb->pid;
    }
    set_pcb_memory_bounds(pcb, start, start + size - 1);

    printf("Allocated memory for PID %d from %d to %d\n", pcb->pid, start, start + size - 1);
    return start;
}

// ✅ تحرير الذاكرة كلها بتاعت العملية
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
    printf("Deallocated memory for PID %d\n", pcb->pid);
}

// ✅ الكتابة في عنوان معين
void write_memory(Memory* memory, int address, const char* name, const char* data, int process_id) {
    if (!memory || address < 0 || address >= MEMORY_SIZE) return;

    free(memory->words[address].name);
    free(memory->words[address].data);

    memory->words[address].name = name ? strdup(name) : NULL;
    memory->words[address].data = data ? strdup(data) : NULL;
    memory->words[address].process_id = process_id;
}

// ✅ قراءة من عنوان معين
void read_memory(const Memory* memory, int address, char** name, char** data, int* process_id) {
    if (!memory || address < 0 || address >= MEMORY_SIZE) return;

    if (name) *name = memory->words[address].name ? strdup(memory->words[address].name) : NULL;
    if (data) *data = memory->words[address].data ? strdup(memory->words[address].data) : NULL;
    if (process_id) *process_id = memory->words[address].process_id;
}

// ✅ طباعة محتويات الذاكرة
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

// ✅ التأكد من وجود مساحة كفاية
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