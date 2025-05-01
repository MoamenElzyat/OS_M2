#ifndef PCB_H
#define PCB_H

#include <stdbool.h>

#define MAX_PROGRAM_NAME_LENGTH 256
#define MAX_VARIABLES 100

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
    int pid;                    // Process ID
    char program_name[MAX_PROGRAM_NAME_LENGTH];
    ProcessState state;         // Current state
    int priority;               // Current priority (1-4, 1 being highest)
    int program_counter;        // Current instruction pointer
    int memory_lower_bound;     // Lower memory boundary
    int memory_upper_bound;     // Upper memory boundary
    int arrival_time;           // Time when process arrives
    int quantum_remaining;      // Remaining quantum time
    int time_in_queue;          // Time spent in ready queue
    char** variables;           // Process variables (names)
    int var_count;              // Number of variables
    char** instructions;        // Program instructions
    int instruction_count;      // Number of instructions
} PCB;

// Function declarations
PCB* create_pcb(int pid, int arrival_time);
void destroy_pcb(PCB* pcb);
void set_pcb_state(PCB* pcb, ProcessState state);
void set_pcb_priority(PCB* pcb, int priority);
void set_pcb_memory_bounds(PCB* pcb, int lower, int upper);
void add_pcb_variable(PCB* pcb, const char* name, const char* value);
void add_pcb_instruction(PCB* pcb, const char* instruction);
const char* get_state_string(ProcessState state);

#endif // PCB_H 