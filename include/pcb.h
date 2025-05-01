#ifndef PCB_H
#define PCB_H

#include <stdbool.h>

#define MAX_PROGRAM_NAME_LENGTH 256

// Process states
typedef enum {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} ProcessState;

// Process Control Block structure
typedef struct {
    int pid;
    char program_name[MAX_PROGRAM_NAME_LENGTH];
    ProcessState state;
    int priority;
    int program_counter;
    int memory_lower_bound;
    int memory_upper_bound;
    int arrival_time;
    int quantum_remaining;
    int time_in_queue;
    char** variables;   // أسماء المتغيرات
    char** values;      // قيم المتغيرات
    int var_count;
    char** instructions;
    int instruction_count;
} PCB;

// Function declarations
PCB* create_pcb(int pid, int arrival_time);
void destroy_pcb(PCB* pcb);
void set_pcb_state(PCB* pcb, ProcessState state);
void set_pcb_priority(PCB* pcb, int priority);
void set_pcb_memory_bounds(PCB* pcb, int lower, int upper);
void add_pcb_instruction(PCB* pcb, const char* instruction);
void update_pcb_variable(PCB* pcb, const char* name, const char* value);
const char* get_pcb_variable(PCB* pcb, const char* name);
const char* get_state_string(ProcessState state);

#endif // PCB_H