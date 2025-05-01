#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "pcb.h"
#include "memory.h"
#include "mutex.h"

// Instruction types
typedef enum {
    INSTR_PRINT,
    INSTR_ASSIGN,
    INSTR_WRITE_FILE,
    INSTR_READ_FILE,
    INSTR_PRINT_FROM_TO,
    INSTR_SEM_WAIT,
    INSTR_SEM_SIGNAL,
    INSTR_UNKNOWN
} InstructionType;

// Parse instruction
InstructionType parse_instruction(const char* instruction);

// Execute instruction (GUI-aware: wraps core logic)
// ✅ New: returns PCB* (unblocked process if any), success via pointer
PCB* execute_instruction(PCB* pcb, Memory* memory, ResourceManager* resources, bool* success);

// Execute core logic
PCB* execute_instruction_core(PCB* pcb, Memory* memory, ResourceManager* resources, bool* success);

// Load program from file
bool load_program(PCB* pcb, const char* filename);

// Debug print of instruction
void print_instruction(const char* instruction);

#endif  // INTERPRETER_H