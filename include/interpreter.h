#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "pcb.h"
#include "memory.h"
#include "mutex.h"
#include "logger.h"

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

InstructionType parse_instruction(const char* instruction);

PCB* execute_instruction(PCB* pcb, Memory* memory, ResourceManager* resources, Logger* logger, bool* success);

PCB* execute_instruction_core(PCB* pcb, Memory* memory, ResourceManager* resources, Logger* logger, bool* success);

bool load_program(PCB* pcb, const char* filename);

#endif