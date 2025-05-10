#ifndef GLOBALS_H
#define GLOBALS_H

// The variable 'scheduler_initialized' is used as a flag to indicate whether the scheduler has been initialized (1 = initialized, 0 = not yet).

#include "scheduler.h"
#include "memory.h"
#include "mutex.h"

extern Scheduler* scheduler;
// Flag to indicate if the scheduler has been initialized (1 = initialized, 0 = not yet)
extern int scheduler_initialized;
extern Memory memory;
extern ResourceManager resource_manager;
extern Logger logger;
extern char gui_input_buffer[256];
extern int gui_input_ready;
void set_last_log(const char* msg);


#endif  // GLOBALS_H