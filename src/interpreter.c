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

bool pcb_has_instruction(PCB* pcb, const char* keyword) {
    for (int i = 0; i < pcb->instruction_count; i++) {
        if (strncmp(pcb->instructions[i], keyword, strlen(keyword)) == 0) {
            return true;
        }
    }
    return false;
}

ResourceType parse_resource(const char* token) {
    if (strcmp(token, "userInput") == 0) return RESOURCE_USER_INPUT;
    if (strcmp(token, "userOutput") == 0) return RESOURCE_USER_OUTPUT;
    if (strcmp(token, "file") == 0) return RESOURCE_FILE;
    return NUM_RESOURCES;
}

PCB* execute_instruction_core(PCB* pcb, Memory* memory, ResourceManager* resources, bool* success) {
    if (!pcb || !memory || !resources) {
        if (success) *success = false;
        return NULL;
    }
    if (pcb->program_counter >= pcb->instruction_count) {
        if (success) *success = false;
        return NULL;
    }

    const char* instruction = pcb->instructions[pcb->program_counter];
    printf("➡️ [Program %d | PID %d] Executing: %s\n", pcb->pid, pcb->pid, instruction);

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
                printf("🖨️ Printing: %s\n", val ? val : tokens[1]);
            }
            break;

        case INSTR_ASSIGN:
            if (token_count < 3) {
                *success = false;
            } else if (strcmp(tokens[2], "input") == 0) {
                // ✅ تحديد الرسائل حسب البرنامج
                if (pcb->pid == 1) {
                    if (strcmp(tokens[1], "a") == 0) {
                        printf("🔢 [INPUT Program 1]: Enter START number for [%s]: ", tokens[1]);
                    } else if (strcmp(tokens[1], "b") == 0) {
                        printf("🔢 [INPUT Program 1]: Enter END number for [%s]: ", tokens[1]);
                    } else {
                        printf("🔢 [INPUT Program 1]: Enter VALUE for [%s]: ", tokens[1]);
                    }
                } else if (pcb->pid == 2) {
                    if (strcmp(tokens[1], "a") == 0) {
                        printf("📝 [INPUT Program 2]: Enter the FILENAME to write to: ");
                    } else if (strcmp(tokens[1], "b") == 0) {
                        printf("🔧 [INPUT Program 2]: Enter the DATA to write inside the file: ");
                    } else {
                        printf("🔧 [INPUT Program 2]: Enter VALUE for [%s]: ", tokens[1]);
                    }
                } else if (pcb->pid == 3) {
                    if (strcmp(tokens[1], "a") == 0) {
                        printf("📝 [INPUT Program 3]: Enter the FILENAME to read from: ");
                    } else {
                        printf("🔧 [INPUT Program 3]: Enter VALUE for [%s]: ", tokens[1]);
                    }
                } else {
                    printf("🔧 [INPUT]: Enter VALUE for [%s]: ", tokens[1]);
                }

                char input_val[256];
                if (fgets(input_val, sizeof(input_val), stdin)) {
                    input_val[strcspn(input_val, "\n")] = 0;
                    update_pcb_variable(pcb, tokens[1], input_val);
                    printf("✅ Assigned [%s] = [%s]\n", tokens[1], input_val);
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
                            printf("📖 READ [%s] into [%s]: [%s]\n", filename, tokens[1], content);
                        } else {
                            printf("⚠️ File [%s] is empty or failed to read.\n", filename);
                        }
                        fclose(file);
                    } else {
                        printf("❌ Cannot open file: %s\n", filename);
                    }
                }
            } else {
                const char* val = get_pcb_variable(pcb, tokens[2]);
                update_pcb_variable(pcb, tokens[1], val ? val : tokens[2]);
                printf("✅ Assigned [%s] = [%s]\n", tokens[1], val ? val : tokens[2]);
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
                    printf("📝 WROTE to [%s]: [%s]\n", filename, data);
                } else {
                    printf("❌ Failed to write to [%s]\n", filename);
                }
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
                printf("🔢 Range [%d to %d]: ", start, end);
                for (int i = start; i <= end; i++) printf("%d ", i);
                printf("\n");
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
                printf("❓ Unknown resource: %s\n", tokens[1]);
                *success = false;
                break;
            }
            if (type == INSTR_SEM_WAIT) {
                bool ok = sem_wait(resources, res, pcb);
                if (!ok) {
                    printf("🔒 [PID %d] BLOCKED on [%s]\n", pcb->pid, tokens[1]);
                    *success = false;
                    free(instruction_copy);
                    return NULL;
                }
                printf("🔓 [PID %d] ACQUIRED [%s]\n", pcb->pid, tokens[1]);
            } else {
                unblocked = sem_signal(resources, res, pcb);
                printf("🔓 [PID %d] RELEASED [%s]\n", pcb->pid, tokens[1]);
                if (unblocked) {
                    printf("🚀 [PID %d] UNBLOCKED and ready.\n", unblocked->pid);
                }
            }
            break;
        }

        default:
            printf("⚠️ Unknown instruction: %s\n", tokens[0]);
            *success = false;
            break;
    }

    free(instruction_copy);
    if (*success) pcb->program_counter++;
    return unblocked;
}

PCB* execute_instruction(PCB* pcb, Memory* memory, ResourceManager* resources, bool* success) {
#ifdef USE_GUI
    // GUI integration hook if needed
#endif
    return execute_instruction_core(pcb, memory, resources, success);
}

bool load_program(PCB* pcb, const char* filename) {
    if (!pcb || !filename) return false;
    FILE* file = fopen(filename, "r");
    if (!file) return false;
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

void print_instruction(const char* instruction) {
    if (!instruction) return;
    printf("➡️ Executing: %s\n", instruction);
}