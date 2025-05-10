#include "globals.h"

Memory memory;
ResourceManager resource_manager;
Logger logger;

char gui_input_buffer[256] = "";
int gui_input_ready = 0;
Scheduler* scheduler = NULL;
int scheduler_initialized = 0;


#include <stdio.h>

__attribute__((constructor))
static void print_globals_init_status() {
    printf("[INIT CHECK] Globals initialized:\n");
    printf("  -> Scheduler pointer: %s\n", scheduler == NULL ? "NULL" : "Initialized");
    printf("  -> GUI Input Buffer: (empty='%s')\n", gui_input_buffer);
    printf("  -> GUI Input Ready Flag: %d\n", gui_input_ready);
}
