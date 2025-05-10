// scheduler_api.h
#ifndef SCHEDULER_API_H
#define SCHEDULER_API_H

#include "scheduler.h"

void api_init_scheduler(SchedulingAlgorithm algo, int quantum);
void reset_scheduler();
void step_execution();
int get_clock_cycle();
const char* get_algorithm_name();
const char* get_process_list();
const char* get_queue_state();
const char* get_memory_state();
const char* get_mutex_state();
int get_total_processes();
int load_process_from_file(const char* path, int arrival_time);  
const char* get_latest_log();  
const char* get_purpose_msg();  // NEW

#endif // SCHEDULER_API_H