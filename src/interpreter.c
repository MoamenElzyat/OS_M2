#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/interpreter.h"

#ifdef USE_GUI
#include <gtk/gtk.h>
#endif

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

void print_input_prompt(int pid, const char* var_name) {
    if (pid == 1) {
        if (strcmp(var_name, "a") == 0)
            printf("🔢 [INPUT Program 1]: Enter START number for [%s]: ", var_name);
        else if (strcmp(var_name, "b") == 0)
            printf("🔢 [INPUT Program 1]: Enter END number for [%s]: ", var_name);
        else
            printf("🔢 [INPUT Program 1]: Enter VALUE for [%s]: ", var_name);
    } else if (pid == 2) {
        if (strcmp(var_name, "a") == 0)
            printf("📝 [INPUT Program 2]: Enter FILENAME to write to: ");
        else if (strcmp(var_name, "b") == 0)
            printf("🔧 [INPUT Program 2]: Enter DATA to write: ");
        else
            printf("🔧 [INPUT Program 2]: Enter VALUE for [%s]: ", var_name);
    } else if (pid == 3) {
        if (strcmp(var_name, "a") == 0)
            printf("📝 [INPUT Program 3]: Enter FILENAME to read from: ");
        else
            printf("🔧 [INPUT Program 3]: Enter VALUE for [%s]: ", var_name);
    } else {
        printf("🔧 [INPUT]: Enter VALUE for [%s]: ", var_name);
    }
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
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg),
             "➡️ [Program %d | PID %d] Executing: %s",
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
    *success = true;
    PCB* unblocked = NULL;

    switch (type) {
        case INSTR_PRINT:
            if (token_count < 2) {
                *success = false;
            } else {
                const char* val = get_pcb_variable(pcb, tokens[1]);
                snprintf(log_msg, sizeof(log_msg), "🖨️ Printing: %s", val ? val : tokens[1]);
                log_event(logger, log_msg);
                printf("🖨️ Printing: %s\n", val ? val : tokens[1]);
            }
            break;

        case INSTR_ASSIGN:
            if (token_count < 3) {
                *success = false;
            } else if (strcmp(tokens[2], "input") == 0) {
                print_input_prompt(pcb->pid, tokens[1]);
                char input_val[256];
                if (fgets(input_val, sizeof(input_val), stdin)) {
                    input_val[strcspn(input_val, "\n")] = 0;
                    update_pcb_variable(pcb, tokens[1], input_val);
                    snprintf(log_msg, sizeof(log_msg),
                             "✅ Assigned [%s] = [%s]", tokens[1], input_val);
                    log_event(logger, log_msg);
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
                                     "📖 READ [%s] into [%s]: %s", filename, tokens[1], content);
                            log_event(logger, log_msg);
                        } else {
                            snprintf(log_msg, sizeof(log_msg),
                                     "⚠️ File [%s] is empty or unreadable.", filename);
                            log_event(logger, log_msg);
                        }
                        fclose(file);
                    } else {
                        snprintf(log_msg, sizeof(log_msg),
                                 "❌ Cannot open file: %s", filename);
                        log_event(logger, log_msg);
                    }
                }
            } else {
                const char* val = get_pcb_variable(pcb, tokens[2]);
                update_pcb_variable(pcb, tokens[1], val ? val : tokens[2]);
                snprintf(log_msg, sizeof(log_msg),
                         "✅ Assigned [%s] = [%s]", tokens[1], val ? val : tokens[2]);
                log_event(logger, log_msg);
            }
            break;

        case INSTR_PRINT_FROM_TO:
            if (token_count < 3) {
                *success = false;
            } else {
                const char* val1 = get_pcb_variable(pcb, tokens[1]);
                const char* val2 = get_pcb_variable(pcb, tokens[2]);
                int start = atoi(val1 ? val1 : tokens[1]);
                int end = atoi(val2 ? val2 : tokens[2]);
                snprintf(log_msg, sizeof(log_msg),
                         "🔢 [PID %d] printFromTo from %d to %d:", pcb->pid, start, end);
                log_event(logger, log_msg);
                printf("🔢 Range [%d to %d]: ", start, end);
                for (int i = start; i <= end; i++) {
                    printf("%d ", i);
                }
                printf("\n");
            }
            break;

        case INSTR_WRITE_FILE:
            if (token_count < 3) {
                *success = false;
            } else {
                const char* filename = get_pcb_variable(pcb, tokens[1]);
                const char* data = get_pcb_variable(pcb, tokens[2]);
                if (!filename) filename = tokens[1];
                if (!data) data = tokens[2];
                FILE* file = fopen(filename, "w");
                if (file) {
                    fprintf(file, "%s\n", data);
                    fclose(file);
                    snprintf(log_msg, sizeof(log_msg),
                             "📝 [PID %d] WROTE to [%s]: %s", pcb->pid, filename, data);
                    log_event(logger, log_msg);
                    printf("📝 WROTE to [%s]: %s\n", filename, data);
                } else {
                    snprintf(log_msg, sizeof(log_msg),
                             "❌ Failed to write to [%s]", filename);
                    log_event(logger, log_msg);
                }
            }
            break;

        case INSTR_SEM_WAIT:
        case INSTR_SEM_SIGNAL: {
            if (token_count < 2) {
                *success = false;
                break;
            }
            ResourceType res = parse_resource(tokens[1]);
            if (res == NUM_RESOURCES) {
                snprintf(log_msg, sizeof(log_msg), "❓ Unknown resource: %s", tokens[1]);
                log_event(logger, log_msg);
                *success = false;
                break;
            }
            if (type == INSTR_SEM_WAIT) {
                bool ok = sem_wait(resources, res, pcb, logger);
                if (!ok) {
                    *success = false;
                    free(instruction_copy);
                    return NULL;
                }
            } else {
                unblocked = sem_signal(resources, res, pcb, logger);
            }
            break;
        }

        default:
            snprintf(log_msg, sizeof(log_msg), "⚠️ Unknown instruction: %s", tokens[0]);
            log_event(logger, log_msg);
            *success = false;
            break;
    }

    free(instruction_copy);
    if (*success) pcb->program_counter++;
    return unblocked;
}

PCB* execute_instruction(PCB* pcb, Memory* memory, ResourceManager* resources, Logger* logger, bool* success) {
#ifdef USE_GUI
#endif
    return execute_instruction_core(pcb, memory, resources, logger, success);
}

bool load_program(PCB* pcb, const char* filename) {
    if (!pcb || !filename) return false;
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("❌ Failed to open program file: %s\n", filename);
        return false;
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;
        add_pcb_instruction(pcb, line);
        printf("📥 [Program %d] Loaded instruction: [%s]\n", pcb->pid, line);
    }
    fclose(file);
    return true;
}