#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/interpreter.h"

// Parse instruction type
InstructionType parse_instruction(const char* instruction) {
    if (!instruction) return INSTR_UNKNOWN;

    if (strncmp(instruction, "print", 5) == 0) return INSTR_PRINT;
    if (strncmp(instruction, "assign", 6) == 0) return INSTR_ASSIGN;
    if (strncmp(instruction, "writeFile", 9) == 0) return INSTR_WRITE_FILE;
    if (strncmp(instruction, "readFile", 8) == 0) return INSTR_READ_FILE;
    if (strncmp(instruction, "printFromTo", 11) == 0) return INSTR_PRINT_FROM_TO;
    if (strncmp(instruction, "semWait", 7) == 0) return INSTR_SEM_WAIT;
    if (strncmp(instruction, "semSignal", 9) == 0) return INSTR_SEM_SIGNAL;

    return INSTR_UNKNOWN;
}

// Execute a single instruction
bool execute_instruction(PCB* pcb, Memory* memory, ResourceManager* resources) {
    if (!pcb || !memory || !resources) return false;

    // Get current instruction
    if (pcb->program_counter >= pcb->instruction_count) return false;
    const char* instruction = pcb->instructions[pcb->program_counter];
    
    // Parse instruction
    InstructionType type = parse_instruction(instruction);
    
    // Tokenize instruction
    char* tokens[3] = {NULL, NULL, NULL};
    char* instruction_copy = strdup(instruction);
    char* token = strtok(instruction_copy, " ");
    int token_count = 0;
    
    while (token && token_count < 3) {
        tokens[token_count++] = token;
        token = strtok(NULL, " ");
    }

    bool success = true;
    
    switch (type) {
        case INSTR_PRINT:
            if (token_count < 2) {
                success = false;
                break;
            }
            // Acquire output resource
            if (!sem_wait(resources, RESOURCE_USER_OUTPUT, pcb)) {
                set_pcb_state(pcb, BLOCKED);
                return false;
            }
            printf("%s\n", tokens[1]);
            sem_signal(resources, RESOURCE_USER_OUTPUT, pcb);
            break;

        case INSTR_ASSIGN:
            if (token_count < 3) {
                success = false;
                break;
            }
            if (strcmp(tokens[2], "input") == 0) {
                // Acquire input resource
                if (!sem_wait(resources, RESOURCE_USER_INPUT, pcb)) {
                    set_pcb_state(pcb, BLOCKED);
                    return false;
                }
                printf("Please enter a value: ");
                char input[256];
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = 0;  // Remove newline
                add_pcb_variable(pcb, tokens[1], input);
                sem_signal(resources, RESOURCE_USER_INPUT, pcb);
            } else {
                add_pcb_variable(pcb, tokens[1], tokens[2]);
            }
            break;

        case INSTR_WRITE_FILE:
            if (token_count < 3) {
                success = false;
                break;
            }
            // Acquire file resource
            if (!sem_wait(resources, RESOURCE_FILE, pcb)) {
                set_pcb_state(pcb, BLOCKED);
                return false;
            }
            FILE* file = fopen(tokens[1], "w");
            if (file) {
                fprintf(file, "%s\n", tokens[2]);
                fclose(file);
            }
            sem_signal(resources, RESOURCE_FILE, pcb);
            break;

        case INSTR_READ_FILE:
            if (token_count < 2) {
                success = false;
                break;
            }
            // Acquire file resource
            if (!sem_wait(resources, RESOURCE_FILE, pcb)) {
                set_pcb_state(pcb, BLOCKED);
                return false;
            }
            FILE* read_file = fopen(tokens[1], "r");
            if (read_file) {
                char content[256];
                if (fgets(content, sizeof(content), read_file)) {
                    content[strcspn(content, "\n")] = 0;  // Remove newline
                    add_pcb_variable(pcb, "file_content", content);
                }
                fclose(read_file);
            }
            sem_signal(resources, RESOURCE_FILE, pcb);
            break;

        case INSTR_PRINT_FROM_TO:
            if (token_count < 3) {
                success = false;
                break;
            }
            // Acquire output resource
            if (!sem_wait(resources, RESOURCE_USER_OUTPUT, pcb)) {
                set_pcb_state(pcb, BLOCKED);
                return false;
            }
            int start = atoi(tokens[1]);
            int end = atoi(tokens[2]);
            for (int i = start; i <= end; i++) {
                printf("%d ", i);
            }
            printf("\n");
            sem_signal(resources, RESOURCE_USER_OUTPUT, pcb);
            break;

        case INSTR_SEM_WAIT:
            if (token_count < 2) {
                success = false;
                break;
            }
            ResourceType resource;
            if (strcmp(tokens[1], "userInput") == 0) resource = RESOURCE_USER_INPUT;
            else if (strcmp(tokens[1], "userOutput") == 0) resource = RESOURCE_USER_OUTPUT;
            else if (strcmp(tokens[1], "file") == 0) resource = RESOURCE_FILE;
            else {
                success = false;
                break;
            }
            if (!sem_wait(resources, resource, pcb)) {
                set_pcb_state(pcb, BLOCKED);
                return false;
            }
            break;

        case INSTR_SEM_SIGNAL:
            if (token_count < 2) {
                success = false;
                break;
            }
            ResourceType signal_resource;
            if (strcmp(tokens[1], "userInput") == 0) signal_resource = RESOURCE_USER_INPUT;
            else if (strcmp(tokens[1], "userOutput") == 0) signal_resource = RESOURCE_USER_OUTPUT;
            else if (strcmp(tokens[1], "file") == 0) signal_resource = RESOURCE_FILE;
            else {
                success = false;
                break;
            }
            sem_signal(resources, signal_resource, pcb);
            break;

        default:
            success = false;
            break;
    }

    free(instruction_copy);
    
    if (success) {
        pcb->program_counter++;
    }
    
    return success;
}

// Load program from file
bool load_program(PCB* pcb, const char* filename) {
    if (!pcb || !filename) return false;

    FILE* file = fopen(filename, "r");
    if (!file) return false;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline
        line[strcspn(line, "\n")] = 0;
        // Skip empty lines
        if (strlen(line) == 0) continue;
        add_pcb_instruction(pcb, line);
    }

    fclose(file);
    return true;
}

// Print instruction
void print_instruction(const char* instruction) {
    if (!instruction) return;
    printf("Executing: %s\n", instruction);
} 