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

// Function declarations
InstructionType parse_instruction(const char* instruction);
bool execute_instruction(PCB* pcb, Memory* memory, ResourceManager* resources);
bool load_program(PCB* pcb, const char* filename);
void print_instruction(const char* instruction);

#endif // INTERPRETER_H 