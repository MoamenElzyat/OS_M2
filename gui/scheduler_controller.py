#!/usr/bin/env python3

import faulthandler
faulthandler.enable()

import sys
import ctypes
import os
import logging
import platform
from PyQt5.QtCore import QTimer
from PyQt5.QtWidgets import QApplication, QTableWidgetItem, QMessageBox, QInputDialog
from scheduler_ui import SchedulerUI
from PyQt5.QtWidgets import QFileDialog



class SchedulerController:
    
    
    def __init__(self):
        self.initialized = False
        self.no_process_warning_shown = False
        self.ui = SchedulerUI()
        self.has_loaded_process = False

        lib_name = None
        if platform.system() == "Darwin":
            lib_name = 'libmyscheduler.dylib'
        elif platform.system() == "Linux":
            lib_name = 'libmyscheduler.so'
        elif platform.system() == "Windows":
            lib_name = 'myscheduler.dll'
        else:
            QMessageBox.critical(None, "Error", f"Unsupported OS: {platform.system()}")
            sys.exit(1)

        lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../bin', lib_name))

        try:
            self.lib = ctypes.CDLL(lib_path)
        except Exception as e:
            QMessageBox.critical(self.ui, "Error", f"Failed to load backend library: {e}")
            sys.exit(1)
            
            
            
        self.lib.has_pending_processes.restype = ctypes.c_int
        
        self.lib.get_purpose_msg.restype = ctypes.c_char_p

        # Set function prototypes
        self.lib.api_init_scheduler.argtypes = [ctypes.c_int, ctypes.c_int]
        self.lib.api_init_scheduler.restype = None
        self.lib.reset_scheduler.restype = None
        self.lib.step_execution.restype = None
        self.lib.get_clock_cycle.restype = ctypes.c_int
        self.lib.get_total_processes.restype = ctypes.c_int
        self.lib.get_algorithm_name.restype = ctypes.c_char_p

        self.lib.get_process_list.restype = ctypes.c_char_p
        self.lib.get_queue_state.restype = ctypes.c_char_p
        self.lib.get_memory_state.restype = ctypes.c_char_p
        self.lib.get_mutex_state.restype = ctypes.c_char_p

        self.lib.load_process_from_file.argtypes = [ctypes.c_char_p, ctypes.c_int]
        self.lib.load_process_from_file.restype = None
        
        self.lib.get_latest_log.restype = ctypes.c_char_p

        #  setup for set_gui_input
        self.lib.is_waiting_for_gui_input.restype = ctypes.c_int
        self.lib.set_gui_input.argtypes = [ctypes.c_char_p]
        self.lib.set_gui_input.restype = None

        # Connect buttons
        self.ui.start_btn.clicked.connect(self.start_simulation)
        self.ui.stop_btn.clicked.connect(self.stop_simulation)
        self.ui.reset_btn.clicked.connect(self.reset_simulation)
        self.ui.step_btn.clicked.connect(self.step_execution)
        self.ui.auto_btn.clicked.connect(self.auto_execution)
        self.ui.add_proc_btn.clicked.connect(self.add_process)
        
        self.ui.load_all_btn.clicked.connect(self.load_all_processes)

        # Timer for auto-update
        self.timer = QTimer()
        self.timer.timeout.connect(self.step_execution)

        # Set up logging
        logging.basicConfig(level=logging.INFO, filename='scheduler_gui.log')
        self.last_seen_log = ""
        
        # Clear logs + tables
        self.ui.execution_log.clear()
        self.ui.process_table.setRowCount(0)
        self.ui.queue_table.setRowCount(0)
        self.ui.memory_table.setRowCount(0)
        self.ui.resource_table.setRowCount(0)

    def start_simulation(self):
        if self.lib.get_total_processes() == 0 and self.lib.has_pending_processes() == 0:
            if not self.no_process_warning_shown:
                self.ui.append_execution_log("Warning", "-", " No process loaded. Please add a process before starting.")
                self.no_process_warning_shown = True
            self.timer.stop()
            return

        self.no_process_warning_shown = False
        self.ui.step_btn.setEnabled(True)
        self.ui.auto_btn.setEnabled(True)

        algo_text = self.ui.algo_combo.currentText()
        if algo_text == "FCFS":
            algo = 0
        elif algo_text == "Round Robin":
            algo = 1
        else:
            algo = 2        # MLFQ

        print(f"[DEBUG PY] Algorithm passed to backend: {algo}") 
        self.ui.lbl_algorithm.setText(f"Algorithm: {algo_text}")

        quantum = self.ui.quantum_spin.value()
        if algo == 1:
            reaction = f"Algorithm: {algo_text} | Quantum: {quantum}"
        else:
            reaction = f"Algorithm: {algo_text}"

        self.ui.append_execution_log("Start Simulation", "-", reaction)

        try:
            if not self.initialized:
                self.lib.api_init_scheduler(algo, quantum)
                self.initialized = True
                self.ui.append_execution_log("[INFO] Simulation initialized.", "-", "-")
            else:
                self.lib.api_init_scheduler(algo, quantum)
                self.ui.append_execution_log("[INFO] Algorithm/Quantum updated.", "-", "-")

            if not self.timer.isActive():
                self.ui.append_execution_log("[INFO] Auto execution started (500ms per step).", "-", "-")
                self.timer.start(500)

        except Exception as e:
            logging.error(f"Failed to initialize backend: {e}")
            QMessageBox.critical(self.ui, "Error", f"Failed to initialize backend: {e}")
            sys.exit(1)
        
        
    def stop_simulation(self):
        self.ui.append_execution_log("[INFO] Stopping simulation.", "-", "-")
        if self.timer.isActive():
            self.timer.stop()

    def reset_simulation(self):
        self.ui.step_btn.setEnabled(True)
        self.ui.auto_btn.setEnabled(True)
        
        # Clear logs + tables
        self.ui.execution_log.clear()
        self.ui.event_log.clear()
        self.ui.process_table.setRowCount(0)
        self.ui.queue_table.setRowCount(0)
        self.ui.memory_table.setRowCount(0)
        self.ui.resource_table.setRowCount(0)

        self.ui.append_execution_log("[INFO] Resetting simulation.", "-", "-")
        self.ui.append_event_log("Reset", "-", "Scheduler reset + PID counter reset to 1.")

        try:
            self.lib.reset_scheduler()
            self.initialized = False
            self.has_loaded_process = False
            self.update_status()
        except Exception as e:
            logging.error(f"Failed to reset simulation: {e}")
            QMessageBox.critical(self.ui, "Error", f"Failed to reset simulation: {e}")
            
            
            
            

    def step_execution(self):
        if self.lib.get_total_processes() == 0 and self.lib.has_pending_processes() == 0:
            if not self.no_process_warning_shown:
                self.ui.append_execution_log("Warning", "-", " No process loaded. Please add a process first.")
                self.no_process_warning_shown = True
            return

        self.no_process_warning_shown = False
        self.ui.step_btn.setEnabled(True)
        self.ui.auto_btn.setEnabled(True)

        self.ui.append_execution_log(instruction="Execute Step (User Action)", reaction="Step command sent to backend")
        try:
            self.lib.step_execution()
            self.update_status()
        except Exception as e:
            logging.error(f"Failed to execute step: {e}")
            QMessageBox.critical(self.ui, "Error", f"Failed to execute step: {e}")

    def auto_execution(self):
        if not self.timer.isActive():
            self.ui.append_execution_log("[INFO] Auto execution will continue (every 500ms).")
            self.timer.start(500)
        else:
            self.ui.append_execution_log("[INFO] Auto execution is already running.", "-", "-")

    def add_process(self):
        path, _ = QFileDialog.getOpenFileName(self.ui, "Select Process File")
        if path:
            if not self.initialized:
                algo_text = self.ui.algo_combo.currentText()
                if algo_text == "FCFS":
                    algo = 0
                elif algo_text == "Round Robin":
                    algo = 1
                else:
                    algo = 2  # MLFQ

                quantum = self.ui.quantum_spin.value()
                self.lib.api_init_scheduler(algo, quantum)
                self.initialized = True
                self.ui.append_execution_log("[INFO] Scheduler auto-initialized during Add Process.", "-", "-")
            arrival = self.ui.arrival_spin.value()
            process_name = os.path.basename(path)
            self.ui.append_execution_log(instruction="Add Process", reaction=f"{process_name} (Arrival: {arrival})")
            try:
                # Load process to backend
                self.lib.load_process_from_file(ctypes.c_char_p(path.encode()), ctypes.c_int(arrival))
                self.update_status()
                self.has_loaded_process = True
                self.no_process_warning_shown = False
                #self.lib.step_execution()
                self.update_status()

                with open(path, 'r') as file:
                    program_instructions = file.read()

                QMessageBox.information(
                    self.ui,
                    "Process Loaded",
                    f" Successfully loaded process:\n\n"
                    f" **File:** {process_name}\n"
                    f" **Arrival Time:** {arrival}\n\n"
                    f" **Program Instructions:**\n\n{program_instructions}"
                )

            except Exception as e:
                logging.error(f"Failed to add process: {e}")
                QMessageBox.critical(self.ui, "Error", f"Failed to add process: {e}")

    def update_status(self):
        try:
            clock_cycle = self.lib.get_clock_cycle()
            total_processes = self.lib.get_total_processes()
            algorithm = self.lib.get_algorithm_name().decode()

            self.ui.lbl_total_processes.setText(f"Total Processes: {total_processes}")
            self.ui.lbl_clock.setText(f"Clock Cycle: {clock_cycle}")
            self.ui.lbl_algorithm.setText(f"Algorithm: {algorithm}")

            process_list_str = self.lib.get_process_list().decode()
            logging.info(f"Process List Raw: {process_list_str}")
            self.update_table(self.ui.process_table, process_list_str)

            queue_state_str = self.lib.get_queue_state().decode()
            self.update_table(self.ui.queue_table, queue_state_str)

            memory_state_str = self.lib.get_memory_state().decode()
            rows = memory_state_str.strip().split('\n')
            self.ui.memory_table.setRowCount(len(rows))
            for idx, row in enumerate(rows):
                self.ui.memory_table.setItem(idx, 0, QTableWidgetItem(row.strip()))

            mutex_state_str = self.lib.get_mutex_state().decode()
            rows = mutex_state_str.strip().split('\n')
            self.ui.resource_table.setRowCount(len(rows))
            for idx, row in enumerate(rows):
                fields = row.split(',')
                for col_idx, value in enumerate(fields):
                    self.ui.resource_table.setItem(idx, col_idx, QTableWidgetItem(value))

            if self.lib.is_waiting_for_gui_input() == 1:
                self.ui.append_execution_log("Input Required", "-", "[GUI_INPUT] Backend is waiting for user input.")
                self.prompt_user_input()

            latest_log = self.lib.get_latest_log().decode()
            if latest_log and latest_log.strip() and latest_log != self.last_seen_log:
                self.last_seen_log = latest_log 
                if "All processes have completed" in latest_log:
                    self.ui.step_btn.setEnabled(False)
                    self.ui.auto_btn.setEnabled(False)
                    if self.timer.isActive():
                        self.timer.stop()
                
                pid = "-"
                if "PID" in latest_log:
                    import re
                    match = re.search(r'PID\s*=?\s*(\d+)', latest_log)
                    if match:
                        pid = match.group(1)

                if "[GUI_PRINT]" in latest_log or "[GUI_PRINT_FROM_TO]" in latest_log:
                    self.ui.append_execution_log("Output", pid, latest_log)
                elif "[GUI_INPUT]" in latest_log:
                    self.ui.append_execution_log("Input Required", pid, latest_log)
                elif "[GUI_FILE_" in latest_log:
                    self.ui.append_execution_log("File Operation", pid, latest_log)
                elif "[Event]" in latest_log:
                    self.ui.append_execution_log("Event", pid, latest_log)
                    self.ui.append_event_log("Event", pid, latest_log)  
                else:
                    self.ui.append_execution_log("Execution", pid, latest_log)

        except Exception as e:
            logging.error(f"Failed to update status: {e}")
            QMessageBox.critical(self.ui, "Error", f"Failed to update status: {e}")

    def update_table(self, table, data):
        try:
            rows = data.strip().split('\n')
            table.setRowCount(0)
            for row in rows:
                fields = row.split(',')
                row_idx = table.rowCount()
                table.insertRow(row_idx)
                for col_idx, value in enumerate(fields):
                    table.setItem(row_idx, col_idx, QTableWidgetItem(value))
        except Exception as e:
            logging.error(f"Failed to update table: {e}")
            QMessageBox.critical(self.ui, "Error", f"Failed to update table: {e}")

    def send_input_to_backend(self, user_input: str):
        print(f"[DEBUG PY] → Sending to backend: '{user_input}'")
        self.lib.set_gui_input(user_input.encode())
        QTimer.singleShot(0, self.step_execution)

    def prompt_user_input(self, custom_message="Input Value:"):
        latest_log = self.lib.get_latest_log().decode()
        msg = self.lib.get_purpose_msg().decode()
        if not msg:
            msg = latest_log

        pid = "-"
        program_number = "-"
        import re
        match = re.search(r'Program\s*(\d+).*PID\s*(\d+)', latest_log)
        if match:
            program_number = match.group(1)
            pid = match.group(2)
        elif "PID" in latest_log:
            pid_match = re.search(r'PID\s*=?\s*(\d+)', latest_log)
            if pid_match:
                pid = pid_match.group(1)
                program_number = pid  

        combined_message = f"[Program {program_number} | PID {pid}] {msg}"

        text, ok = QInputDialog.getText(self.ui, "Enter Input", combined_message)
        if ok and text:
            self.send_input_to_backend(text)
            
            
            
            
            
    def load_all_processes(self):
        algo = 2          # 0=FCFS, 1=RR, 2=MLFQ
        self.ui.algo_combo.setCurrentText("MLFQ")

        quantum = self.ui.quantum_spin.value() or 1  

        self.lib.api_init_scheduler(algo, quantum)
        if not self.initialized:
            self.ui.append_execution_log("[INFO] Scheduler initialized for scenario (MLFQ).", "-", "-")
        else:
            self.ui.append_execution_log("[INFO] Scheduler re‑initialized for scenario (MLFQ).", "-", "-")
        self.initialized = True

        files = [
            ("program3.txt", 0),
            ("program1.txt", 1),
            ("program2.txt", 3)
        ]
        for fname, arrival in files:
            path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', fname))
            self.lib.load_process_from_file(ctypes.c_char_p(path.encode()), ctypes.c_int(arrival))
            self.ui.append_execution_log("Add Process", "-", f"{fname} (Arrival: {arrival})")
        self.no_process_warning_shown = False
        self.update_status()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    controller = SchedulerController()
    controller.ui.show()
    sys.exit(app.exec_())