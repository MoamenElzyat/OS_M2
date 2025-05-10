#include "scheduler_api.h"
#include "pcb.h"
#include "memory.h"
#include "mutex.h"
#include "scheduler.h"
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "interpreter.h"
#include "queue.h"
#include "scheduler.h"
#include <stdlib.h>

static char process_list_buffer[2048];
static char queue_state_buffer[2048];
static char memory_state_buffer[2048];
static char mutex_state_buffer[2048];
static char last_log[512] = "";  
int already_initialized = 0;
extern char purpose_msg[256];
static int next_pid = 1;

void set_last_log(const char* msg) {  
    strncpy(last_log, msg, sizeof(last_log)-1);
    last_log[sizeof(last_log)-1] = '\0';
}

const char* get_latest_log() {  
    return last_log;
}

void api_init_scheduler(SchedulingAlgorithm algorithm, int quantum) {
    printf("[DEBUG C] api_init_scheduler called with algorithm=%d, quantum=%d\n", algorithm, quantum);

    if (!scheduler_initialized) {
        printf("[AUTO-INIT] Scheduler was not initialized, performing initialization now.\n");
    }
    if (already_initialized && scheduler_initialized) {
        printf("[INFO] Scheduler already initialized. Skipping re-init.\n");
        log_event(&logger, "Scheduler already initialized. Skipping re-init.");
        return;
    }
    scheduler = malloc(sizeof(Scheduler));
    if (!scheduler) {
        printf("[FATAL] Failed to allocate Scheduler!\n");
        exit(1);
    }
    init_scheduler(scheduler, algorithm, quantum);
    init_memory(&memory);
    init_resource_manager(&resource_manager);
    scheduler_initialized = 1;

    log_event(&logger, "Scheduler fully initialized with new algorithm and quantum.");
    already_initialized = 1;
}

const char* get_process_list() {
    printf("[DEBUG] get_process_list: scheduler=%p\n", scheduler);
    if (scheduler == NULL) {
        printf("[FATAL] scheduler is NULL inside get_process_list!\n");
        return "SCHEDULER_NULL";
    }
    memset(process_list_buffer, 0, sizeof(process_list_buffer));

    //  Ready queues 
    for (int lvl = 0; lvl < 4; lvl++) {
        ProcessQueue* queue = &scheduler->ready_queues[lvl];
        for (int i = 0; i < queue->size; i++) {
            PCB* pcb = queue->processes[i];
            char line[128];
            snprintf(line, sizeof(line), "%d,%s,%d,%d-%d,%d\n",
                pcb->pid,
                get_state_string(pcb->state),
                pcb->priority,
                pcb->memory_lower_bound, pcb->memory_upper_bound,
                pcb->program_counter);
            if (strlen(process_list_buffer) + strlen(line) + 1 < sizeof(process_list_buffer)) {
                strncat(process_list_buffer, line, sizeof(process_list_buffer) - strlen(process_list_buffer) - 1);
            }
        }
    }

    //  Blocked queue
    ProcessQueue* blocked = &scheduler->blocked_queue;
    for (int i = 0; i < blocked->size; i++) {
        PCB* pcb = blocked->processes[i];
        char line[128];
        snprintf(line, sizeof(line), "%d,%s,%d,%d-%d,%d\n",
            pcb->pid,
            get_state_string(pcb->state),
            pcb->priority,
            pcb->memory_lower_bound, pcb->memory_upper_bound,
            pcb->program_counter);
        if (strlen(process_list_buffer) + strlen(line) + 1 < sizeof(process_list_buffer)) {
            strncat(process_list_buffer, line, sizeof(process_list_buffer) - strlen(process_list_buffer) - 1);
        }
    }

    //  Running process
    if (scheduler->running_process) {
        PCB* pcb = scheduler->running_process;
        char line[128];
        snprintf(line, sizeof(line), "%d,%s,%d,%d-%d,%d (RUNNING)\n",
            pcb->pid,
            get_state_string(pcb->state),
            pcb->priority,
            pcb->memory_lower_bound, pcb->memory_upper_bound,
            pcb->program_counter);
        if (strlen(process_list_buffer) + strlen(line) + 1 < sizeof(process_list_buffer)) {
            strncat(process_list_buffer, line, sizeof(process_list_buffer) - strlen(process_list_buffer) - 1);
        }
    }

    //  Pending processes
    for (int i = 0; i < pending_list.count; i++) {
        PCB* pcb = pending_list.list[i];
        char line[128];
        snprintf(line, sizeof(line), "%d,%s,%d,%d-%d,%d (PENDING)\n",
            pcb->pid,
            "PENDING",
            pcb->priority,
            pcb->memory_lower_bound, pcb->memory_upper_bound,
            pcb->program_counter);
        if (strlen(process_list_buffer) + strlen(line) + 1 < sizeof(process_list_buffer)) {
            strncat(process_list_buffer, line, sizeof(process_list_buffer) - strlen(process_list_buffer) - 1);
        }
    }

    return process_list_buffer;
}

const char* get_queue_state() {
    printf("[DEBUG] get_queue_state: scheduler=%p\n", scheduler);
    if (scheduler == NULL) {
        printf("[FATAL] scheduler is NULL inside get_queue_state!\n");
        return "SCHEDULER_NULL";
    }
    memset(queue_state_buffer, 0, sizeof(queue_state_buffer));

    for (int lvl = 0; lvl < 4; lvl++) {
        ProcessQueue* queue = &scheduler->ready_queues[lvl];
        for (int i = 0; i < queue->size; i++) {
            PCB* pcb = queue->processes[i];
            char line[128];
            snprintf(line, sizeof(line), "ReadyQ%d: %d,%d,%d\n",
                lvl,
                pcb->pid, pcb->program_counter, pcb->time_in_queue);
            if (strlen(queue_state_buffer) + strlen(line) + 1 < sizeof(queue_state_buffer)) {
                strncat(queue_state_buffer, line, sizeof(queue_state_buffer) - strlen(queue_state_buffer) - 1);
            }
        }
    }

    ProcessQueue* blocked = &scheduler->blocked_queue;
    for (int i = 0; i < blocked->size; i++) {
        PCB* pcb = blocked->processes[i];
        char line[128];
        snprintf(line, sizeof(line), "Blocked: %d,%d,%d\n",
            pcb->pid, pcb->program_counter, pcb->time_in_queue);
        if (strlen(queue_state_buffer) + strlen(line) + 1 < sizeof(queue_state_buffer)) {
            strncat(queue_state_buffer, line, sizeof(queue_state_buffer) - strlen(queue_state_buffer) - 1);
        }
    }

    return queue_state_buffer;
}

const char* get_memory_state() {
    printf("[DEBUG] get_memory_state: scheduler=%p\n", scheduler);
    if (scheduler == NULL) {
        printf("[FATAL] scheduler is NULL inside get_memory_state!\n");
        return "SCHEDULER_NULL";
    }
    memset(memory_state_buffer, 0, sizeof(memory_state_buffer));

    for (int i = 0; i < MEMORY_SIZE; i++) {
        char* name = NULL;
        char* data = NULL;
        int pid = -1;
        read_memory(&memory, i, &name, &data, &pid);

        char line[256];  

        if (data != NULL && pid > 0) {
            snprintf(line, sizeof(line), "%d: %s (PID=%d)\n", i, data, pid);
        } else {
            snprintf(line, sizeof(line), "%d: EMPTY (PID=0)\n", i);  
        }

        if (strlen(memory_state_buffer) + strlen(line) + 1 < sizeof(memory_state_buffer)) {
            strncat(memory_state_buffer, line, sizeof(memory_state_buffer) - strlen(memory_state_buffer) - 1);
        }
    }

    return memory_state_buffer;
}

const char* get_mutex_state() {
    printf("[DEBUG] get_mutex_state: scheduler=%p\n", scheduler);
    if (scheduler == NULL) {
        printf("[FATAL] scheduler is NULL inside get_mutex_state!\n");
        return "SCHEDULER_NULL";
    }
    memset(mutex_state_buffer, 0, sizeof(mutex_state_buffer));

    snprintf(mutex_state_buffer, sizeof(mutex_state_buffer),
        "UserInput: held_by=%d, waiting=%d\n"
        "UserOutput: held_by=%d, waiting=%d\n"
        "File: held_by=%d, waiting=%d\n",
        resource_manager.mutexes[RESOURCE_USER_INPUT].owner_pid, resource_manager.mutexes[RESOURCE_USER_INPUT].queue_size,
        resource_manager.mutexes[RESOURCE_USER_OUTPUT].owner_pid, resource_manager.mutexes[RESOURCE_USER_OUTPUT].queue_size,
        resource_manager.mutexes[RESOURCE_FILE].owner_pid, resource_manager.mutexes[RESOURCE_FILE].queue_size);

    return mutex_state_buffer;
}

void reset_scheduler() {
    if (scheduler != NULL) {
        destroy_scheduler(scheduler);
    } else {
        printf("[WARN] Tried to destroy scheduler but it was NULL.\n");
    }
    printf("[TRACE] destroy_scheduler finished, memory freed.\n");
    if (scheduler == NULL) {
        printf("[FATAL] scheduler is NULL inside reset_scheduler before init_scheduler!\n");
        return;
    }
    init_scheduler(scheduler, scheduler->algorithm, scheduler->quantum);
    if (scheduler == NULL) {
        printf("[FATAL] scheduler is NULL inside reset_scheduler before print_queues_state!\n");
        return;
    }
    print_queues_state(scheduler);
    init_memory(&memory);
    init_resource_manager(&resource_manager);
    set_last_log("Scheduler reset.");
    already_initialized = 0;
}

void step_execution() {
    if (scheduler == NULL) {
        printf("[FATAL] scheduler is NULL inside step_execution!\n");
        return;
    }
    scheduler_step();
}

int get_clock_cycle() {
    if (scheduler == NULL) {
        printf("[FATAL] scheduler is NULL inside get_clock_cycle!\n");
        return -1;
    }
    return scheduler->clock_cycle;
}

int get_total_processes() {
    if (scheduler == NULL) {
        printf("[FATAL] scheduler is NULL inside get_total_processes!\n");
        return -1;
    }
    int total = 0;
    for (int lvl = 0; lvl < 4; lvl++) {
        total += scheduler->ready_queues[lvl].size;
    }
    total += scheduler->blocked_queue.size;
    if (scheduler->running_process != NULL) total++;
    return total;
}

const char* get_algorithm_name() {
    if (scheduler == NULL) {
        printf("[FATAL] scheduler is NULL inside get_algorithm_name!\n");
        return "SCHEDULER_NULL";
    }
    switch (scheduler->algorithm) {
        case FCFS: return "FCFS";
        case RR: return "Round Robin";
        case MLFQ: return "MLFQ";
        default: return "Unknown";
    }
}

int load_process_from_file(const char* path, int arrival_time) {  
    PCB* pcb = create_pcb(next_pid++, arrival_time);

    const char* filename = strrchr(path, '/');
    if (filename) filename++; else filename = path;
    strncpy(pcb->program_name, filename, sizeof(pcb->program_name));

    if (!load_program(&memory, pcb, path)) {
        printf("Failed to load program from %s\n", path);
        destroy_pcb(pcb);
        set_last_log("Failed to load process.");  
        return -1;
    }

    if (pcb->priority < 1) {
        pcb->priority = 1;
    }

    printf("Process loaded from %s (PID: %d)\n", path, pcb->pid);
    pending_list.list[pending_list.count++] = pcb;
    already_initialized = 1;

    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Loaded process from %s (PID: %d)", filename, pcb->pid); 
    set_last_log(log_msg);
    return pcb->pid;
}

int has_pending_processes() {
    return pending_list.count > 0;
}

const char* get_purpose_msg() {
    return purpose_msg;
}