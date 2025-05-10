#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

#define MAX_LOG_LINES 100
#define MAX_LOG_LENGTH 256

typedef struct {
    char execution_log[MAX_LOG_LINES][MAX_LOG_LENGTH];
    char event_log[MAX_LOG_LINES][MAX_LOG_LENGTH];
    int execution_count;
    int event_count;
    FILE* log_file;  
} Logger;

void init_logger(Logger* logger);
void log_execution(Logger* logger, int pid, const char* instruction);
void log_event(Logger* logger, const char* message);
void print_logs(Logger* logger);
void destroy_logger(Logger* logger);  
const char* get_latest_log();
void set_last_log(const char* msg);
#endif