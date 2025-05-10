#include "logger.h"
#include "globals.h"
#include <string.h>
#include <stdio.h>
#include "interpreter.h"
#include "memory.h"
#include "mutex.h"
#include "pcb.h"
#include "queue.h"
#include "scheduler.h"


void init_logger(Logger* logger) {
    logger->execution_count = 0;
    logger->event_count = 0;
    logger->log_file = fopen("logs.txt", "w");
    if (!logger->log_file) {
        printf("Failed to open logs.txt for writing!\n");
    }
}

void log_execution(Logger* logger, int pid, const char* instruction) {
    if (logger->execution_count < MAX_LOG_LINES) {
        snprintf(logger->execution_log[logger->execution_count++],
                MAX_LOG_LENGTH, "[PID %d] Executing: %s", pid, instruction);
    }
    if (logger->log_file) {
        fprintf(logger->log_file, "[PID %d] Executing: %s\n", pid, instruction);
        fflush(logger->log_file);
    }
}

void log_event(Logger* logger, const char* msg) {
    if (!msg) return;

    if (logger->log_file) {
        fprintf(logger->log_file, "%s\n", msg);
        fflush(logger->log_file);
    }


    set_last_log(msg);  
}

void print_logs(Logger* logger) {
    FILE* file = fopen("logs.txt", "w");
    if (!file) {
        printf("Failed to open logs.txt for writing!\n");
        return;
    }

    fprintf(file, "\n=== EXECUTION LOG ===\n");
    for (int i = 0; i < logger->execution_count; i++) {
        fprintf(file, "%s\n", logger->execution_log[i]);
    }

    fprintf(file, "\n=== EVENT MESSAGES ===\n");
    for (int i = 0; i < logger->event_count; i++) {
        fprintf(file, "%s\n", logger->event_log[i]);
    }

    fclose(file);

    printf("\n=== EXECUTION LOG ===\n");
    for (int i = 0; i < logger->execution_count; i++) {
        printf("%s\n", logger->execution_log[i]);
    }

    printf("\n=== EVENT MESSAGES ===\n");
    for (int i = 0; i < logger->event_count; i++) {
        printf("%s\n", logger->event_log[i]);
    }

    printf("\nLogs saved to logs.txt\n");
}

void destroy_logger(Logger* logger) {
    if (logger->log_file) {
        fclose(logger->log_file);
        logger->log_file = NULL;
    }
}