#include <stdio.h>
#include <assert.h>
#include "../include/interpreter.h"
#include "../include/memory.h"
#include "../include/mutex.h"
#include "../include/pcb.h"

void test_parse_instruction() {
    assert(parse_instruction("print x") == INSTR_PRINT);
    assert(parse_instruction("assign x y") == INSTR_ASSIGN);
    assert(parse_instruction("writeFile x y") == INSTR_WRITE_FILE);
    assert(parse_instruction("readFile x") == INSTR_READ_FILE);
    assert(parse_instruction("printFromTo x y") == INSTR_PRINT_FROM_TO);
    assert(parse_instruction("semWait file") == INSTR_SEM_WAIT);
    assert(parse_instruction("semSignal file") == INSTR_SEM_SIGNAL);
    assert(parse_instruction("unknown") == INSTR_UNKNOWN);

    printf("test_parse_instruction passed.\n");
}

void test_execute_simple_print() {
    Memory memory;
    ResourceManager resources;
    init_memory(&memory);
    init_resource_manager(&resources);

    PCB* pcb = create_pcb(1, 0);
    add_pcb_instruction(pcb, "print Hello");  // ✅ فقط print

    // Simulate running
    bool res = execute_instruction_core(pcb, &memory, &resources);
    assert(res == true);

    destroy_pcb(pcb);
    printf("test_execute_simple_print passed.\n");
}

void test_interpreter_invalid_instruction() {
    PCB* pcb = create_pcb(1, 0);
    add_pcb_instruction(pcb, "invalid_cmd x y");

    Memory memory;
    init_memory(&memory);

    ResourceManager resources;
    init_resource_manager(&resources);

    bool res = execute_instruction_core(pcb, &memory, &resources);  // ✅ خليها core
    assert(!res && "Invalid instruction should fail execution");

    printf("test_interpreter_invalid_instruction passed.\n");
    destroy_pcb(pcb);
}

int main() {
    test_parse_instruction();
    test_execute_simple_print();
    test_interpreter_invalid_instruction();  // ✅ أضفتها
    printf("All interpreter tests passed.\n");
    return 0;
}