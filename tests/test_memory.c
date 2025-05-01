#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "../include/memory.h"
#include "../include/pcb.h"

void test_memory_allocation() {
    Memory memory;
    PCB* pcb = create_pcb(1, 0);
    init_memory(&memory);

    int start = allocate_memory(&memory, pcb, 5);
    assert(start != -1);
    assert(pcb->memory_lower_bound == start);
    assert(pcb->memory_upper_bound == start + 4);

    // Check that memory slots are allocated
    for (int i = start; i <= pcb->memory_upper_bound; i++) {
        assert(memory.words[i].process_id == pcb->pid);
    }

    deallocate_memory(&memory, pcb);
    // Check that memory is freed
    for (int i = 0; i < MEMORY_SIZE; i++) {
        assert(memory.words[i].process_id == -1);
    }

    destroy_pcb(pcb);
    printf("test_memory_allocation passed.\n");
}

void test_memory_write_read() {
    Memory memory;
    init_memory(&memory);

    write_memory(&memory, 0, "x", "10", 1);

    char* name;
    char* data;
    int pid;
    read_memory(&memory, 0, &name, &data, &pid);

    assert(strcmp(name, "x") == 0);
    assert(strcmp(data, "10") == 0);
    assert(pid == 1);

    free(name);
    free(data);

    printf("test_memory_write_read passed.\n");
}
void test_memory_overflow() {
    Memory memory;
    init_memory(&memory);

    PCB pcb;
    pcb.pid = 1;

    int result = allocate_memory(&memory, &pcb, MEMORY_SIZE + 1);
    assert(result == -1 && "Memory overflow should fail allocation");

    printf("test_memory_overflow passed.\n");
}

int main() {
    test_memory_allocation();
    test_memory_write_read();
    printf("All memory tests passed.\n");
    return 0;
}