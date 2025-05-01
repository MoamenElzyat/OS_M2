#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>
#include "scheduler.h"
#include "memory.h"
#include "mutex.h"

// GUI Components
typedef struct {
    GtkWidget *window;
    GtkWidget *main_box;
    
    // Overview Section
    GtkWidget *overview_frame;
    GtkWidget *process_count_label;
    GtkWidget *clock_cycle_label;
    GtkWidget *algorithm_label;
    
    // Process List
    GtkWidget *process_frame;
    GtkWidget *process_view;
    GtkListStore *process_store;
    
    // Queue Section
    GtkWidget *queue_frame;
    GtkWidget *ready_queue_view;
    GtkWidget *blocked_queue_view;
    GtkWidget *running_process_view;
    GtkListStore *ready_queue_store;
    GtkListStore *blocked_queue_store;
    GtkListStore *running_process_store;
    
    // Scheduler Control Panel
    GtkWidget *control_frame;
    GtkWidget *algorithm_combo;
    GtkWidget *start_button;
    GtkWidget *stop_button;
    GtkWidget *reset_button;
    GtkWidget *quantum_spin;
    GtkWidget *step_button;
    GtkWidget *auto_button;
    
    // Resource Management Panel
    GtkWidget *resource_frame;
    GtkWidget *mutex_view;
    GtkWidget *resource_blocked_view;
    GtkListStore *mutex_store;
    GtkListStore *resource_blocked_store;
    
    // Memory Viewer
    GtkWidget *memory_frame;
    GtkWidget *memory_view;
    GtkListStore *memory_store;
    
    // Log & Console Panel
    GtkWidget *log_frame;
    GtkWidget *log_view;
    GtkTextBuffer *log_buffer;
    GtkWidget *status_bar;
    
    // Process Creation Panel
    GtkWidget *process_creation_frame;
    GtkWidget *file_chooser_button;
    GtkWidget *arrival_time_spin;
    GtkWidget *add_process_button;
    
    // Data references
    Scheduler *scheduler;
    Memory *memory;
    ResourceManager *resources;
    
    // Execution state
    gboolean is_running;
    guint auto_timer_id;
} GUI;

// Window dimensions
#define MAIN_WINDOW_WIDTH 1200
#define MAIN_WINDOW_HEIGHT 800

// Function declarations
void init_gui(int *argc, char ***argv);
void cleanup_gui(void);
void create_gui(Scheduler *scheduler, Memory *memory, ResourceManager *resources);
void update_gui(void);
void show_gui(void);

// Callback declarations
void on_start_clicked(GtkButton *button, gpointer user_data);
void on_stop_clicked(GtkButton *button, gpointer user_data);
void on_reset_clicked(GtkButton *button, gpointer user_data);
void on_step_clicked(GtkButton *button, gpointer user_data);
void on_auto_clicked(GtkButton *button, gpointer user_data);
void on_algorithm_changed(GtkComboBox *combo, gpointer user_data);
void on_add_process_clicked(GtkButton *button, gpointer user_data);
void on_file_chooser_clicked(GtkButton *button, gpointer user_data);
void on_window_destroy(GtkWidget *widget, gpointer user_data);

// Helper functions
void update_overview(void);
void update_process_list(void);
void update_queue_views(void);
void update_resource_views(void);
void update_memory_view(void);
void update_log(const char* message);
void clear_log(void);
void reset_simulation(void);

#endif // GUI_H 