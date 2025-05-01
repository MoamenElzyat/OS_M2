#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/gui.h"
#include "../include/interpreter.h"

// Global GUI instance
static GUI gui;

// Initialize GUI
void init_gui(int *argc, char ***argv) {
    gtk_init(argc, argv);
    gui.is_running = FALSE;
    gui.auto_timer_id = 0;
}

// Cleanup GUI
void cleanup_gui(void) {
    if (gui.auto_timer_id) {
        g_source_remove(gui.auto_timer_id);
    }
    gtk_main_quit();
}

// Create process list columns
static void create_process_columns(void) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // PID Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("PID", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(gui.process_view), column);
    
    // State Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("State", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(gui.process_view), column);
    
    // Priority Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Priority", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(gui.process_view), column);
    
    // Memory Boundaries Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Memory", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(gui.process_view), column);
    
    // PC Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("PC", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(gui.process_view), column);
}

// Create queue columns
static void create_queue_columns(GtkWidget *view, const char* columns[]) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    for (int i = 0; columns[i] != NULL; i++) {
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(columns[i], renderer, "text", i, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    }
}

// Create memory columns
static void create_memory_columns(void) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // Address Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Addr", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(gui.memory_view), column);
    
    // Content Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Content", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(gui.memory_view), column);
    
    // Process Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Process", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(gui.memory_view), column);
}

// Create GUI
void create_gui(Scheduler *scheduler, Memory *memory, ResourceManager *resources) {
    // Store references to data
    gui.scheduler = scheduler;
    gui.memory = memory;
    gui.resources = resources;
    
    // Create main window
    gui.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gui.window), "OS Scheduler Simulation");
    gtk_window_set_default_size(GTK_WINDOW(gui.window), MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
    g_signal_connect(gui.window, "destroy", G_CALLBACK(on_window_destroy), NULL);
    
    // Create main vertical box
    gui.main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(gui.window), gui.main_box);
    
    // Create overview section
    gui.overview_frame = gtk_frame_new("System Overview");
    GtkWidget *overview_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(gui.overview_frame), overview_box);
    
    gui.process_count_label = gtk_label_new("Processes: 0");
    gui.clock_cycle_label = gtk_label_new("Clock Cycle: 0");
    gui.algorithm_label = gtk_label_new("Algorithm: FCFS");
    
    gtk_box_pack_start(GTK_BOX(overview_box), gui.process_count_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(overview_box), gui.clock_cycle_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(overview_box), gui.algorithm_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(gui.main_box), gui.overview_frame, FALSE, FALSE, 0);
    
    // Create horizontal box for main content
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(gui.main_box), content_box, TRUE, TRUE, 0);
    
    // Create left panel (Process List and Queues)
    GtkWidget *left_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(content_box), left_panel, TRUE, TRUE, 0);
    
    // Process List
    gui.process_frame = gtk_frame_new("Process List");
    gui.process_store = gtk_list_store_new(5, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
    gui.process_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(gui.process_store));
    create_process_columns();
    
    GtkWidget *process_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(process_scroll), gui.process_view);
    gtk_container_add(GTK_CONTAINER(gui.process_frame), process_scroll);
    gtk_box_pack_start(GTK_BOX(left_panel), gui.process_frame, TRUE, TRUE, 0);
    
    // Queue Section
    gui.queue_frame = gtk_frame_new("Queues");
    GtkWidget *queue_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(gui.queue_frame), queue_box);
    
    // Ready Queue
    GtkWidget *ready_frame = gtk_frame_new("Ready Queue");
    gui.ready_queue_store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
    gui.ready_queue_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(gui.ready_queue_store));
    const char *ready_columns[] = {"PID", "Instruction", "Time", NULL};
    create_queue_columns(gui.ready_queue_view, ready_columns);
    
    GtkWidget *ready_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(ready_scroll), gui.ready_queue_view);
    gtk_container_add(GTK_CONTAINER(ready_frame), ready_scroll);
    gtk_box_pack_start(GTK_BOX(queue_box), ready_frame, TRUE, TRUE, 0);
    
    // Running Process
    GtkWidget *running_frame = gtk_frame_new("Running Process");
    gui.running_process_store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
    gui.running_process_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(gui.running_process_store));
    create_queue_columns(gui.running_process_view, ready_columns);
    
    GtkWidget *running_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(running_scroll), gui.running_process_view);
    gtk_container_add(GTK_CONTAINER(running_frame), running_scroll);
    gtk_box_pack_start(GTK_BOX(queue_box), running_frame, TRUE, TRUE, 0);
    
    // Blocked Queue
    GtkWidget *blocked_queue_frame = gtk_frame_new("Blocked Queue");
    gui.blocked_queue_store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
    gui.blocked_queue_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(gui.blocked_queue_store));
    create_queue_columns(gui.blocked_queue_view, ready_columns);
    
    GtkWidget *blocked_queue_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(blocked_queue_scroll), gui.blocked_queue_view);
    gtk_container_add(GTK_CONTAINER(blocked_queue_frame), blocked_queue_scroll);
    gtk_box_pack_start(GTK_BOX(queue_box), blocked_queue_frame, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(left_panel), gui.queue_frame, TRUE, TRUE, 0);
    
    // Create right panel (Control, Resources, Memory)
    GtkWidget *right_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(content_box), right_panel, TRUE, TRUE, 0);
    
    // Control Panel
    gui.control_frame = gtk_frame_new("Scheduler Control");
    GtkWidget *control_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(gui.control_frame), control_box);
    
    // Algorithm selection
    GtkWidget *algo_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *algo_label = gtk_label_new("Algorithm:");
    gui.algorithm_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(gui.algorithm_combo), "FCFS");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(gui.algorithm_combo), "Round Robin");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(gui.algorithm_combo), "MLFQ");
    gtk_combo_box_set_active(GTK_COMBO_BOX(gui.algorithm_combo), 0);
    g_signal_connect(gui.algorithm_combo, "changed", G_CALLBACK(on_algorithm_changed), NULL);
    
    gtk_box_pack_start(GTK_BOX(algo_box), algo_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(algo_box), gui.algorithm_combo, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(control_box), algo_box, FALSE, FALSE, 0);
    
    // Quantum control
    GtkWidget *quantum_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *quantum_label = gtk_label_new("Quantum:");
    gui.quantum_spin = gtk_spin_button_new_with_range(1, 10, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(gui.quantum_spin), 2);
    
    gtk_box_pack_start(GTK_BOX(quantum_box), quantum_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quantum_box), gui.quantum_spin, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(control_box), quantum_box, FALSE, FALSE, 0);
    
    // Control buttons
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gui.start_button = gtk_button_new_with_label("Start");
    gui.stop_button = gtk_button_new_with_label("Stop");
    gui.reset_button = gtk_button_new_with_label("Reset");
    gui.step_button = gtk_button_new_with_label("Step");
    gui.auto_button = gtk_button_new_with_label("Auto");
    
    g_signal_connect(gui.start_button, "clicked", G_CALLBACK(on_start_clicked), NULL);
    g_signal_connect(gui.stop_button, "clicked", G_CALLBACK(on_stop_clicked), NULL);
    g_signal_connect(gui.reset_button, "clicked", G_CALLBACK(on_reset_clicked), NULL);
    g_signal_connect(gui.step_button, "clicked", G_CALLBACK(on_step_clicked), NULL);
    g_signal_connect(gui.auto_button, "clicked", G_CALLBACK(on_auto_clicked), NULL);
    
    gtk_box_pack_start(GTK_BOX(button_box), gui.start_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), gui.stop_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), gui.reset_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), gui.step_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), gui.auto_button, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(control_box), button_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(right_panel), gui.control_frame, FALSE, FALSE, 0);
    
    // Resource Management Panel
    gui.resource_frame = gtk_frame_new("Resource Management");
    GtkWidget *resource_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(gui.resource_frame), resource_box);
    
    // Mutex status
    GtkWidget *mutex_frame = gtk_frame_new("Mutex Status");
    gui.mutex_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
    gui.mutex_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(gui.mutex_store));
    const char *mutex_columns[] = {"Resource", "Status", "Owner", NULL};
    create_queue_columns(gui.mutex_view, mutex_columns);
    
    GtkWidget *mutex_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(mutex_scroll), gui.mutex_view);
    gtk_container_add(GTK_CONTAINER(mutex_frame), mutex_scroll);
    gtk_box_pack_start(GTK_BOX(resource_box), mutex_frame, TRUE, TRUE, 0);
    
    // Blocked processes
    GtkWidget *resource_blocked_frame = gtk_frame_new("Blocked Processes");
    gui.resource_blocked_store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
    gui.resource_blocked_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(gui.resource_blocked_store));
    create_queue_columns(gui.resource_blocked_view, ready_columns);
    
    GtkWidget *resource_blocked_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(resource_blocked_scroll), gui.resource_blocked_view);
    gtk_container_add(GTK_CONTAINER(resource_blocked_frame), resource_blocked_scroll);
    gtk_box_pack_start(GTK_BOX(resource_box), resource_blocked_frame, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(right_panel), gui.resource_frame, TRUE, TRUE, 0);
    
    // Memory Viewer
    gui.memory_frame = gtk_frame_new("Memory");
    gui.memory_store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
    gui.memory_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(gui.memory_store));
    create_memory_columns();
    
    GtkWidget *memory_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(memory_scroll), gui.memory_view);
    gtk_container_add(GTK_CONTAINER(gui.memory_frame), memory_scroll);
    gtk_box_pack_start(GTK_BOX(right_panel), gui.memory_frame, TRUE, TRUE, 0);
    
    // Process Creation Panel
    gui.process_creation_frame = gtk_frame_new("Process Creation");
    GtkWidget *creation_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(gui.process_creation_frame), creation_box);
    
    gui.file_chooser_button = gtk_file_chooser_button_new("Select Program File", GTK_FILE_CHOOSER_ACTION_OPEN);
    gui.arrival_time_spin = gtk_spin_button_new_with_range(0, 100, 1);
    gui.add_process_button = gtk_button_new_with_label("Add Process");
    
    g_signal_connect(gui.file_chooser_button, "file-set", G_CALLBACK(on_file_chooser_clicked), NULL);
    g_signal_connect(gui.add_process_button, "clicked", G_CALLBACK(on_add_process_clicked), NULL);
    
    gtk_box_pack_start(GTK_BOX(creation_box), gui.file_chooser_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(creation_box), gui.arrival_time_spin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(creation_box), gui.add_process_button, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(right_panel), gui.process_creation_frame, FALSE, FALSE, 0);
    
    // Log Panel
    gui.log_frame = gtk_frame_new("Execution Log");
    gui.log_view = gtk_text_view_new();
    gui.log_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gui.log_view));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(gui.log_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(gui.log_view), FALSE);
    
    GtkWidget *log_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(log_scroll), gui.log_view);
    gtk_container_add(GTK_CONTAINER(gui.log_frame), log_scroll);
    gtk_box_pack_start(GTK_BOX(gui.main_box), gui.log_frame, FALSE, FALSE, 0);
    
    // Status bar
    gui.status_bar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(gui.main_box), gui.status_bar, FALSE, FALSE, 0);
    
    // Set padding
    gtk_container_set_border_width(GTK_CONTAINER(gui.window), 10);
}

// Update overview section
void update_overview(void) {
    char buf[128];
    
    // Update process count
    int total_processes = gui.scheduler->ready_queues[0].size +
                         gui.scheduler->ready_queues[1].size +
                         gui.scheduler->ready_queues[2].size +
                         gui.scheduler->ready_queues[3].size +
                         (gui.scheduler->running_process ? 1 : 0);
    sprintf(buf, "Processes: %d", total_processes);
    gtk_label_set_text(GTK_LABEL(gui.process_count_label), buf);
    
    // Update clock cycle
    sprintf(buf, "Clock Cycle: %d", gui.scheduler->clock_cycle);
    gtk_label_set_text(GTK_LABEL(gui.clock_cycle_label), buf);
    
    // Update algorithm
    const char *algo_name;
    switch (gui.scheduler->algorithm) {
        case FCFS: algo_name = "First Come First Serve"; break;
        case RR: algo_name = "Round Robin"; break;
        case MLFQ: algo_name = "Multi-Level Feedback Queue"; break;
        default: algo_name = "Unknown"; break;
    }
    sprintf(buf, "Algorithm: %s", algo_name);
    gtk_label_set_text(GTK_LABEL(gui.algorithm_label), buf);
}

// Update process list
void update_process_list(void) {
    gtk_list_store_clear(gui.process_store);
    GtkTreeIter iter;
    
    // Add running process
    if (gui.scheduler->running_process) {
        gtk_list_store_append(gui.process_store, &iter);
        gtk_list_store_set(gui.process_store, &iter,
                          0, gui.scheduler->running_process->pid,
                          1, get_state_string(gui.scheduler->running_process->state),
                          2, gui.scheduler->running_process->priority,
                          3, "0-59", // Memory boundaries would be calculated
                          4, gui.scheduler->running_process->program_counter,
                          -1);
    }
    
    // Add processes in ready queues
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < gui.scheduler->ready_queues[i].size; j++) {
            PCB* pcb = gui.scheduler->ready_queues[i].processes[j];
            gtk_list_store_append(gui.process_store, &iter);
            gtk_list_store_set(gui.process_store, &iter,
                             0, pcb->pid,
                             1, get_state_string(pcb->state),
                             2, pcb->priority,
                             3, "0-59", // Memory boundaries would be calculated
                             4, pcb->program_counter,
                             -1);
        }
    }
}

// Update blocked queue
void update_blocked_queue(void) {
    gtk_list_store_clear(gui.blocked_queue_store);
    GtkTreeIter iter;
    
    for (int i = 0; i < NUM_RESOURCES; i++) {
        for (int j = 0; j < gui.resources->mutexes[i].queue_size; j++) {
            PCB* pcb = gui.resources->mutexes[i].waiting_queue[j];
            gtk_list_store_append(gui.blocked_queue_store, &iter);
            gtk_list_store_set(gui.blocked_queue_store, &iter,
                             0, pcb->pid,
                             1, pcb->instructions[pcb->program_counter],
                             2, pcb->time_in_queue,
                             -1);
        }
    }
}

// Update resource blocked processes
void update_resource_blocked_processes(void) {
    gtk_list_store_clear(gui.resource_blocked_store);
    GtkTreeIter iter;
    
    for (int i = 0; i < NUM_RESOURCES; i++) {
        for (int j = 0; j < gui.resources->mutexes[i].queue_size; j++) {
            PCB* pcb = gui.resources->mutexes[i].waiting_queue[j];
            gtk_list_store_append(gui.resource_blocked_store, &iter);
            gtk_list_store_set(gui.resource_blocked_store, &iter,
                             0, pcb->pid,
                             1, pcb->instructions[pcb->program_counter],
                             2, pcb->time_in_queue,
                             -1);
        }
    }
}

// Update queue views
void update_queue_views(void) {
    // Clear all stores
    gtk_list_store_clear(gui.ready_queue_store);
    gtk_list_store_clear(gui.running_process_store);
    
    GtkTreeIter iter;
    
    // Update ready queue
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < gui.scheduler->ready_queues[i].size; j++) {
            PCB* pcb = gui.scheduler->ready_queues[i].processes[j];
            gtk_list_store_append(gui.ready_queue_store, &iter);
            gtk_list_store_set(gui.ready_queue_store, &iter,
                             0, pcb->pid,
                             1, pcb->instructions[pcb->program_counter],
                             2, pcb->time_in_queue,
                             -1);
        }
    }
    
    // Update running process
    if (gui.scheduler->running_process) {
        PCB* pcb = gui.scheduler->running_process;
        gtk_list_store_append(gui.running_process_store, &iter);
        gtk_list_store_set(gui.running_process_store, &iter,
                          0, pcb->pid,
                          1, pcb->instructions[pcb->program_counter],
                          2, pcb->time_in_queue,
                          -1);
    }
    
    // Update blocked queues
    update_blocked_queue();
    update_resource_blocked_processes();
}

// Update resource views
void update_resource_views(void) {
    gtk_list_store_clear(gui.mutex_store);
    gtk_list_store_clear(gui.resource_blocked_store);
    
    GtkTreeIter iter;
    
    // Update mutex status
    for (int i = 0; i < NUM_RESOURCES; i++) {
        gtk_list_store_append(gui.mutex_store, &iter);
        gtk_list_store_set(gui.mutex_store, &iter,
                          0, get_resource_name(i),
                          1, gui.resources->mutexes[i].locked ? "Locked" : "Free",
                          2, gui.resources->mutexes[i].owner_pid,
                          -1);
    }
    
    // Update blocked processes
    for (int i = 0; i < NUM_RESOURCES; i++) {
        for (int j = 0; j < gui.resources->mutexes[i].queue_size; j++) {
            PCB* pcb = gui.resources->mutexes[i].waiting_queue[j];
            gtk_list_store_append(gui.resource_blocked_store, &iter);
            gtk_list_store_set(gui.resource_blocked_store, &iter,
                             0, pcb->pid,
                             1, pcb->instructions[pcb->program_counter],
                             2, pcb->time_in_queue,
                             -1);
        }
    }
}

// Update memory view
void update_memory_view(void) {
    gtk_list_store_clear(gui.memory_store);
    GtkTreeIter iter;
    
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (gui.memory->words[i].process_id != -1) {
            gtk_list_store_append(gui.memory_store, &iter);
            gtk_list_store_set(gui.memory_store, &iter,
                             0, i,
                             1, gui.memory->words[i].data ? gui.memory->words[i].data : "-",
                             2, gui.memory->words[i].process_id,
                             -1);
        }
    }
}

// Update log
void update_log(const char* message) {
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(gui.log_buffer, &iter);
    gtk_text_buffer_insert(gui.log_buffer, &iter, message, -1);
    gtk_text_buffer_insert(gui.log_buffer, &iter, "\n", -1);
    
    // Scroll to the end
    GtkTextMark *mark = gtk_text_buffer_create_mark(gui.log_buffer, "end", &iter, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(gui.log_view), mark, 0.0, FALSE, 0.0, 1.0);
    gtk_text_buffer_delete_mark(gui.log_buffer, mark);
}

// Clear log
void clear_log(void) {
    gtk_text_buffer_set_text(gui.log_buffer, "", -1);
}

// Reset simulation
void reset_simulation(void) {
    // Reset scheduler
    destroy_scheduler(gui.scheduler);
    init_scheduler(gui.scheduler, FCFS, 1);
    
    // Reset memory
    init_memory(gui.memory);
    
    // Reset resources
    init_resource_manager(gui.resources);
    
    // Clear log
    clear_log();
    
    // Update GUI
    update_gui();
}

// Update GUI
void update_gui(void) {
    update_overview();
    update_process_list();
    update_queue_views();
    update_resource_views();
    update_memory_view();
}

// Show GUI
void show_gui(void) {
    gtk_widget_show_all(gui.window);
    gtk_main();
}

// Callback: Start button clicked
void on_start_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    gui.is_running = TRUE;
    gtk_widget_set_sensitive(gui.start_button, FALSE);
    gtk_widget_set_sensitive(gui.stop_button, TRUE);
    gtk_widget_set_sensitive(gui.step_button, FALSE);
    gtk_widget_set_sensitive(gui.auto_button, FALSE);
    update_log("Simulation started");
}

// Callback: Stop button clicked
void on_stop_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    gui.is_running = FALSE;
    gtk_widget_set_sensitive(gui.start_button, TRUE);
    gtk_widget_set_sensitive(gui.stop_button, FALSE);
    gtk_widget_set_sensitive(gui.step_button, TRUE);
    gtk_widget_set_sensitive(gui.auto_button, TRUE);
    update_log("Simulation stopped");
}

// Callback: Reset button clicked
void on_reset_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    reset_simulation();
    update_log("Simulation reset");
}

// Callback: Step button clicked
void on_step_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    update_scheduler(gui.scheduler);
    if (gui.scheduler->running_process) {
        execute_instruction(gui.scheduler->running_process, gui.memory, gui.resources);
        char msg[256];
        sprintf(msg, "Process %d executed instruction: %s",
                gui.scheduler->running_process->pid,
                gui.scheduler->running_process->instructions[gui.scheduler->running_process->program_counter - 1]);
        update_log(msg);
    } else {
        update_log("No running process");
    }
    
    update_gui();
}

// Callback: Auto button clicked
void on_auto_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    if (!gui.auto_timer_id) {
        gui.auto_timer_id = g_timeout_add(1000, (GSourceFunc)on_step_clicked, NULL);
        gtk_widget_set_sensitive(gui.auto_button, FALSE);
        update_log("Auto execution started");
    }
}

// Callback: Algorithm changed
void on_algorithm_changed(GtkComboBox *combo, gpointer user_data) {
    (void)user_data;
    
    int active = gtk_combo_box_get_active(combo);
    switch (active) {
        case 0: gui.scheduler->algorithm = FCFS; break;
        case 1: gui.scheduler->algorithm = RR; break;
        case 2: gui.scheduler->algorithm = MLFQ; break;
    }
    
    int quantum = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(gui.quantum_spin));
    gui.scheduler->quantum = quantum;
    
    update_gui();
    char msg[128];
    sprintf(msg, "Algorithm changed to %s (quantum = %d)",
            active == 0 ? "FCFS" : (active == 1 ? "Round Robin" : "MLFQ"),
            quantum);
    update_log(msg);
}

// Callback: File chooser clicked
void on_file_chooser_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(gui.file_chooser_button));
    if (filename) {
        char msg[256];
        sprintf(msg, "Selected program file: %s", filename);
        update_log(msg);
    }
}

// Callback: Add process clicked
void on_add_process_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(gui.file_chooser_button));
    if (!filename) {
        update_log("Error: No program file selected");
        return;
    }
    
    int arrival_time = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(gui.arrival_time_spin));
    PCB* pcb = create_pcb(gui.scheduler->next_pid++, arrival_time);
    
    if (load_program(pcb, filename)) {
        add_process(gui.scheduler, pcb);
        char msg[256];
        sprintf(msg, "Added process %d from file %s (arrival time: %d)",
                pcb->pid, filename, arrival_time);
        update_log(msg);
        update_gui();
    } else {
        destroy_pcb(pcb);
        update_log("Error: Failed to load program");
    }
}

// Callback: Window destroyed
void on_window_destroy(GtkWidget *widget, gpointer user_data) {
    (void)widget;
    (void)user_data;
    cleanup_gui();
} 