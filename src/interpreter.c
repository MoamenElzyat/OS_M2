#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"
#include "interpreter.h"
#include "memory.h"
#include "mutex.h"
#include "pcb.h"
#include "queue.h"
#include "scheduler.h"  
#include "logger.h"
#include "queue.h"

#ifdef USE_GUI
#include <gtk/gtk.h>
#endif

char purpose_msg[256] = "";

InstructionType parse_instruction(const char* instruction) {
    if (!instruction) return INSTR_UNKNOWN;
    if (strncmp(instruction, "printFromTo", 11) == 0) return INSTR_PRINT_FROM_TO;
    if (strncmp(instruction, "print", 5) == 0) return INSTR_PRINT;
    if (strncmp(instruction, "assign", 6) == 0) return INSTR_ASSIGN;
    if (strncmp(instruction, "writeFile", 9) == 0) return INSTR_WRITE_FILE;
    if (strncmp(instruction, "readFile", 8) == 0) return INSTR_READ_FILE;
    if (strncmp(instruction, "semWait", 7) == 0) return INSTR_SEM_WAIT;
    if (strncmp(instruction, "semSignal", 9) == 0) return INSTR_SEM_SIGNAL;
    return INSTR_UNKNOWN;
}

ResourceType parse_resource(const char* token) {
    if (strcmp(token, "userInput") == 0) return RESOURCE_USER_INPUT;
    if (strcmp(token, "userOutput") == 0) return RESOURCE_USER_OUTPUT;
    if (strcmp(token, "file") == 0) return RESOURCE_FILE;
    return NUM_RESOURCES;
}

PCB* execute_instruction_core(PCB* pcb, Memory* memory, ResourceManager* resources, Logger* logger, bool* success) {
    if (!pcb || !memory || !resources) {
        if (success) *success = false;
        return NULL;
    }
    if (pcb->program_counter >= pcb->instruction_count) {
        if (success) *success = false;
        return NULL;
    }

    const char* instruction = pcb->instructions[pcb->program_counter];
    char log_msg[512];

    snprintf(log_msg, sizeof(log_msg),
            "[Program %d | PID %d] Executing: %s",
            pcb->pid, pcb->pid, instruction);
    log_event(logger, log_msg);

    char* tokens[4] = {NULL, NULL, NULL, NULL};
    char* instruction_copy = strdup(instruction);
    char* token = strtok(instruction_copy, " ");
    int token_count = 0;

    while (token && token_count < 4) {
        tokens[token_count++] = token;
        token = strtok(NULL, " ");
    }

    InstructionType type = parse_instruction(tokens[0]);
    printf("[DEBUG] Instruction Type: %d | Instruction: %s\n", type, instruction);
    *success = true;
    PCB* unblocked = NULL;

    switch (type) {
        case INSTR_PRINT: {
            if (token_count >= 2) {
                const char* val = get_pcb_variable(pcb, tokens[1]);
                snprintf(log_msg, sizeof(log_msg),
                            "[GUI_PRINT] [Program: %s | PID %d] Printing: %s",
                            pcb->program_name, pcb->pid, val ? val : tokens[1]);
                log_event(logger, log_msg);
                printf("Printing: %s\n", val ? val : tokens[1]);
            } else {
                *success = false;
            }
            break;
        }
        case INSTR_ASSIGN: {
            if (token_count >= 3) {
                if (strcmp(tokens[2], "input") == 0) {
                    if (gui_input_ready == 1) {
                        gui_input_ready = 0;
                        update_pcb_variable(pcb, tokens[1], gui_input_buffer);
                        // Sync the updated variable into memory after GUI input is received
                        int var_index = -1;
                        for (int i = 0; i < pcb->var_count; i++) {
                            if (strcmp(pcb->variables[i], tokens[1]) == 0) {
                                var_index = i;
                                break;
                            }
                        }
                        if (var_index == -1) {
                            var_index = pcb->var_count;
                        }
                        int addr = pcb->memory_lower_bound + pcb->instruction_count + var_index;
                        write_memory(memory, addr, tokens[1], gui_input_buffer, pcb->pid);
                        snprintf(log_msg, sizeof(log_msg),
                                 "[GUI_INPUT]  Assigned [%s] = [%s]", tokens[1], gui_input_buffer);
                        log_event(logger, log_msg);
        
                        //int addr = pcb->memory_lower_bound + pcb->program_counter;
                        //write_memory(memory, addr, tokens[1], gui_input_buffer, pcb->pid);
                    } else {
                        gui_input_ready = -1;
                        snprintf(purpose_msg, sizeof(purpose_msg),
                                    "Program: %s (PID %d)\nPlease enter a value for [%s]",
                                    pcb->program_name, pcb->pid, tokens[1]);
        
                        int var_index = -1;
                        for (int i = 0; i < pcb->var_count; i++) {
                            if (strcmp(pcb->variables[i], tokens[1]) == 0) {
                                var_index = i;
                                break;
                            }
                        }
                        if (var_index == -1) {
                            var_index = pcb->var_count;  
                        }
                        int addr = pcb->memory_lower_bound + pcb->instruction_count + var_index;
                        set_pcb_state(pcb, BLOCKED);
                        add_to_queue(&scheduler->blocked_queue, pcb);

                        printf("[DEBUG] Waiting for GUI input - setting success = false\n");
                        *success = false;
                    }
                } else if (strcmp(tokens[2], "readFile") == 0 && token_count == 4) {
                    const char* filename = get_pcb_variable(pcb, tokens[3]);
                    if (filename) {
                        FILE* file = fopen(filename, "r");
                        if (file) {
                            char content[256];
                            if (fgets(content, sizeof(content), file)) {
                                content[strcspn(content, "\n")] = 0;
                                update_pcb_variable(pcb, tokens[1], content);
                                snprintf(log_msg, sizeof(log_msg),
                                            "[Program: %s | PID %d] [GUI_FILE_READ] READ [%s] into [%s]: %s",
                                            pcb->program_name, pcb->pid, filename, tokens[1], content);
                                log_event(logger, log_msg);
        
                                //int addr = pcb->memory_lower_bound + pcb->program_counter;
                                //write_memory(memory, addr, tokens[1], content, pcb->pid);
                            }
                            fclose(file);
                        }
                    }
                } else {
            const char* val = get_pcb_variable(pcb, tokens[2]);
            update_pcb_variable(pcb, tokens[1], val ? val : tokens[2]);
            // Sync variable assignment to memory
            int var_index = -1;
            for (int i = 0; i < pcb->var_count; i++) {
                if (strcmp(pcb->variables[i], tokens[1]) == 0) {
                    var_index = i;
                    break;
                }
            }
            if (var_index == -1) {
                var_index = pcb->var_count;
            }
            int addr = pcb->memory_lower_bound + pcb->instruction_count + var_index;
            write_memory(memory, addr, tokens[1], val ? val : tokens[2], pcb->pid);
            snprintf(log_msg, sizeof(log_msg),
                    " Assigned [%s] = [%s]", tokens[1], val ? val : tokens[2]);
            log_event(logger, log_msg);
        
                    //int addr = pcb->memory_lower_bound + pcb->program_counter;
                    //write_memory(memory, addr, tokens[1], val ? val : tokens[2], pcb->pid);
                }
            } else {
                *success = false;
            }
            break;
        }
        case INSTR_PRINT_FROM_TO:
            if (token_count < 3) {
                *success = false;
            } else {
                const char* val1 = get_pcb_variable(pcb, tokens[1]);
                const char* val2 = get_pcb_variable(pcb, tokens[2]);
                int start = atoi(val1 ? val1 : tokens[1]);
                int end = atoi(val2 ? val2 : tokens[2]);

                // Prepare the full log with numbers
                char range_output[512] = {0};
                snprintf(range_output, sizeof(range_output),
                            "[GUI_PRINT_FROM_TO] [Program: %s | PID %d] Range [%d to %d]: ",
                            pcb->program_name, pcb->pid, start, end);
                for (int i = start; i <= end; i++) {
                    char num[16];
                    snprintf(num, sizeof(num), "%d ", i);
                    strncat(range_output, num, sizeof(range_output) - strlen(range_output) - 1);
                }

                // Log it fully
                log_event(logger, range_output);

                // Also print to console for debug
                printf("%s\n", range_output + strlen("[GUI_PRINT_FROM_TO] "));
            }
            break;
        case INSTR_WRITE_FILE: {
            if (token_count >= 3) {
                const char* filename = get_pcb_variable(pcb, tokens[1]);
                const char* data = get_pcb_variable(pcb, tokens[2]);
                if (!filename) filename = tokens[1];
                if (!data) data = tokens[2];
                FILE* file = fopen(filename, "w");
                if (file) {
                    fprintf(file, "%s\n", data);
                    fclose(file);
                    snprintf(log_msg, sizeof(log_msg),
                            "[Program: %s | PID %d] [GUI_FILE_WRITE]  Wrote to [%s]: %s", pcb->program_name, pcb->pid, filename, data);   
                    log_event(logger, log_msg);
                }
            } else {
                *success = false;
            }
            break;
        }
        case INSTR_SEM_WAIT:
        case INSTR_SEM_SIGNAL: {
            if (token_count >= 2) {
                ResourceType res = parse_resource(tokens[1]);
                if (res != NUM_RESOURCES) {
                    if (type == INSTR_SEM_WAIT) {
                        if (!sem_wait(resources, res, pcb, logger)) {
                            *success = false;
                            free(instruction_copy);
                            return NULL;
                        }
                    } else {
                        unblocked = sem_signal(resources, res, pcb, logger);
                    }
                }
            } else {
                *success = false;
            }
            break;
        }
        default:
            snprintf(log_msg, sizeof(log_msg), " Unknown instruction: %s", tokens[0]);
            log_event(logger, log_msg);
            *success = false;
            break;
    }

    free(instruction_copy);
    if (*success) pcb->program_counter++;
    printf("[DEBUG]  Memory synced for PID %d after execution step.\n", pcb->pid);
    return unblocked;
}

PCB* execute_instruction(PCB* pcb, Memory* memory, ResourceManager* resources, Logger* logger, bool* success) {
    return execute_instruction_core(pcb, memory, resources, logger, success);
}

bool load_program(Memory* memory, PCB* pcb, const char* filename) {
    if (!pcb || !filename || !memory) return false;

    printf("[DEBUG] Opening program file: %s\n", filename);
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf(" Failed to open program file: %s\n", filename);
        return false;
    }

    int instruction_count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strlen(line) > 1) instruction_count++;
    }
    rewind(file);

    printf("[DEBUG] Instruction count: %d\n", instruction_count);

    printf("[DEBUG] Attempting memory allocation for PID %d...\n", pcb->pid);
    int mem_start = allocate_memory(memory, pcb, instruction_count);
    if (mem_start == -1) {
        printf(" Failed to allocate memory for process %d\n", pcb->pid);
        fclose(file);
        return false;
    }

    printf("[DEBUG] Memory allocated at [%d - %d] for PID %d\n",
        pcb->memory_lower_bound, pcb->memory_upper_bound, pcb->pid);

    int index = 0;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;

        add_pcb_instruction(pcb, line);
        printf("[Program %d] Loaded instruction: [%s]\n", pcb->pid, line);

        printf("[DEBUG] Writing to memory address %d (PID %d): %s\n",
            mem_start + index, pcb->pid, line);
        write_memory(memory, mem_start + index, "instruction", line, pcb->pid);
        index++;
    }

    fclose(file);
    printf("[DEBUG] Program %d fully loaded with %d instructions.\n", pcb->pid, pcb->instruction_count);
    return true;
}

void set_gui_input(const char* input) {
    if (input) {
        strncpy(gui_input_buffer, input, sizeof(gui_input_buffer) - 1);
        gui_input_buffer[sizeof(gui_input_buffer) - 1] = '\0';
        gui_input_ready = 1;
        printf("[DEBUG] Received GUI input: %s\n", gui_input_buffer);
        log_event(&logger, "[GUI] Received input from GUI.");

        if (!scheduler) {
            printf("[ERROR] Scheduler is NULL in set_gui_input!\n");
            return;
        }

        // فك البلوك
        for (int i = 0; i < scheduler->blocked_queue.size; ) {
            PCB* blocked_pcb = scheduler->blocked_queue.processes[i];
            if (blocked_pcb->state == BLOCKED) {
                int instr_index = blocked_pcb->program_counter;
                const char* last_instruction = blocked_pcb->instructions[instr_index];

                if (last_instruction) {
                    char instr_copy[256];
                    strncpy(instr_copy, last_instruction, sizeof(instr_copy) - 1);
                    instr_copy[sizeof(instr_copy) - 1] = '\0';
                    char* first_token = strtok(instr_copy, " ");
                    char* third_token = NULL;
                    strtok(NULL, " ");  // skip second token
                    third_token = strtok(NULL, " ");

                    if (first_token && strcmp(first_token, "assign") == 0 && third_token && strcmp(third_token, "input") == 0) {
                        printf("[DEBUG] Unblocking PID %d waiting for GUI input (confirmed assign ... input)\n", blocked_pcb->pid);
                        set_pcb_state(blocked_pcb, READY);
                        add_process(scheduler, blocked_pcb);
                        remove_from_queue(&scheduler->blocked_queue, i);
                        continue;  
                    }
                }
                i++;
            } 
        }
    }
}

int is_waiting_for_gui_input() {
    return gui_input_ready == -1;
}
