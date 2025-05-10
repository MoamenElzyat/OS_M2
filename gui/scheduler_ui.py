import sys
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton,
    QComboBox, QTableWidget, QTableWidgetItem, QTextEdit, QSpinBox, QGroupBox
)
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QDesktopWidget


class SchedulerUI(QMainWindow):
    def __init__(self):
        super().__init__()
        screen = QDesktopWidget().screenGeometry()
        screen_height = screen.height()
        screen_width = screen.width()
        self.setGeometry(0, 0, screen_width, screen_height)
        self.setWindowTitle("Scheduler Simulation GUI")
        self.initUI()
        self.showMaximized()

    def initUI(self):
        main_widget = QWidget()
        main_layout = QHBoxLayout()
        left_panel = QVBoxLayout()
        right_panel = QVBoxLayout()

        # Dashboard
        dashboard_group = QGroupBox("System Overview")
        dashboard_layout = QVBoxLayout()
        self.lbl_total_processes = QLabel("Total Processes: 0")
        self.lbl_clock = QLabel("Clock Cycle: 0")
        self.lbl_algorithm = QLabel("Algorithm: FCFS")
        dashboard_layout.addWidget(self.lbl_total_processes)
        dashboard_layout.addWidget(self.lbl_clock)
        dashboard_layout.addWidget(self.lbl_algorithm)
        dashboard_group.setLayout(dashboard_layout)
        left_panel.addWidget(dashboard_group)

        # Process List
        process_group = QGroupBox("Process List")
        self.process_table = QTableWidget(0, 5)
        self.process_table.setHorizontalHeaderLabels([
            "PID", "State", "Priority", "Memory", "PC"
        ])
        process_layout = QVBoxLayout()
        process_layout.addWidget(self.process_table)
        process_group.setLayout(process_layout)
        left_panel.addWidget(process_group)

        # Queue Section
        queue_group = QGroupBox("Queues")
        self.queue_table = QTableWidget(0, 4)
        self.queue_table.setHorizontalHeaderLabels([
            "Queue", "PID", "Instruction", "Time in Queue"
        ])
        queue_layout = QVBoxLayout()
        queue_layout.addWidget(self.queue_table)
        queue_group.setLayout(queue_layout)
        left_panel.addWidget(queue_group)

        # Resource Management Panel 
        resource_group = QGroupBox("Resource Management")
        resource_layout = QVBoxLayout()
        self.resource_table = QTableWidget(3, 3)
        self.resource_table.setHorizontalHeaderLabels([
            "Resource", "Held By", "Waiting"
        ])
        self.resource_table.setColumnWidth(0, 200)
        self.resource_table.setColumnWidth(1, 100)
        self.resource_table.setColumnWidth(2, 100)
        self.resource_table.setVerticalHeaderLabels([
            "userInput", "userOutput", "file"
        ])
        self.resource_table.setFixedHeight(110)
        resource_layout.addWidget(self.resource_table)
        resource_group.setLayout(resource_layout)
        left_panel.addWidget(resource_group)

        # Scheduler Control Panel
        control_group = QGroupBox("Scheduler Control")
        control_layout = QHBoxLayout()
        self.algo_combo = QComboBox()
        self.algo_combo.addItems(["FCFS", "Round Robin", "MLFQ"])
        self.quantum_spin = QSpinBox()
        self.quantum_spin.setRange(1, 100)
        self.quantum_spin.setValue(2)
        self.quantum_spin.setEnabled(False)
        self.algo_combo.currentTextChanged.connect(self.on_algo_changed)
        self.start_btn = QPushButton("Start")
        self.stop_btn = QPushButton("Stop")
        self.reset_btn = QPushButton("Reset")
        control_layout.addWidget(QLabel("Algorithm:"))
        control_layout.addWidget(self.algo_combo)
        control_layout.addWidget(QLabel("Quantum:"))
        control_layout.addWidget(self.quantum_spin)
        control_layout.addWidget(self.start_btn)
        control_layout.addWidget(self.stop_btn)
        control_layout.addWidget(self.reset_btn)
        control_group.setLayout(control_layout)
        right_panel.addWidget(control_group)

        # Memory + Logs
        mem_log_group = QGroupBox()
        mem_log_layout = QHBoxLayout()

        # Memory Viewer
        memory_group = QGroupBox("Memory Viewer")
        memory_layout = QVBoxLayout()
        self.memory_table = QTableWidget(60, 1)
        self.memory_table.setVerticalHeaderLabels([str(i) for i in range(60)])
        self.memory_table.setHorizontalHeaderLabels(["Instruction"])
        self.memory_table.setColumnWidth(0, 250)
        memory_layout.addWidget(self.memory_table)
        memory_group.setLayout(memory_layout)
        mem_log_layout.addWidget(memory_group)

        # Log & Console Panel
        log_group = QGroupBox("Log & Console Panel")
        log_layout = QVBoxLayout()

        log_layout.addWidget(QLabel("Execution Log"))
        self.execution_log = QTextEdit()
        self.execution_log.setReadOnly(True)
        self.execution_log.setStyleSheet("color: white; background-color: #2b2b2b;")
        log_layout.addWidget(self.execution_log)

        log_layout.addWidget(QLabel("Event Messages"))
        self.event_log = QTextEdit()
        self.event_log.setReadOnly(True)
        self.event_log.setStyleSheet("color: white; background-color: #2b2b2b;")
        log_layout.addWidget(self.event_log)

        log_group.setLayout(log_layout)
        mem_log_layout.addWidget(log_group)

        mem_log_group.setLayout(mem_log_layout)
        right_panel.addWidget(mem_log_group)
        
        
        


        # Process Creation and Config
        proc_ctrl_group = QGroupBox("Process Creation & Config")
        proc_ctrl_layout = QHBoxLayout()
        self.add_proc_btn = QPushButton("Add Process")
        self.arrival_spin = QSpinBox()
        self.arrival_spin.setRange(0, 1000)
        proc_ctrl_layout.addWidget(self.add_proc_btn)
        proc_ctrl_layout.addWidget(QLabel("Arrival Time:"))
        proc_ctrl_layout.addWidget(self.arrival_spin)
        proc_ctrl_group.setLayout(proc_ctrl_layout)
        right_panel.addWidget(proc_ctrl_group)

        # Execution Control
        exec_group = QGroupBox("Execution Control")
        exec_layout = QHBoxLayout()
        self.step_btn = QPushButton("Step")
        self.auto_btn = QPushButton("Auto")
        exec_layout.addWidget(self.step_btn)
        exec_layout.addWidget(self.auto_btn)
        exec_group.setLayout(exec_layout)
        right_panel.addWidget(exec_group)

        main_layout.addLayout(left_panel, 2)
        main_layout.addLayout(right_panel, 3)
        main_widget.setLayout(main_layout)
        self.setCentralWidget(main_widget)
        
        
        
        #cheek mlfq
        self.load_all_btn = QPushButton("Load All Processes")
        exec_layout.addWidget(self.load_all_btn)
        
        
        
        

    def append_execution_log(self, instruction="", pid="-", reaction=""):
        log_entry = f"Instruction: {instruction} | PID: {pid} | Action: {reaction}"
        colored_entry = log_entry
        if "Acquired" in reaction:
            colored_entry = f'<span style="color:green;">{log_entry}</span>'
        elif "Released" in reaction:
            colored_entry = f'<span style="color:blue;">{log_entry}</span>'
        elif "Blocked" in reaction:
            colored_entry = f'<span style="color:red;">{log_entry}</span>'
        elif "Unblocked" in reaction:
            colored_entry = f'<span style="color:purple;">{log_entry}</span>'
        else:
            colored_entry = log_entry
        self.execution_log.append(colored_entry)
        self.execution_log.verticalScrollBar().setValue(self.execution_log.verticalScrollBar().maximum())

    def append_event_message(self, message):
        self.event_log.append(f"<b>Event:</b> {message}")
        self.event_log.verticalScrollBar().setValue(self.event_log.verticalScrollBar().maximum())
        
    def append_event_log(self, instruction, pid, reaction):
        colored_entry = reaction
        if "Acquired" in reaction:
            colored_entry = f'<span style="color:green;">{reaction}</span>'
        elif "Released" in reaction:
            colored_entry = f'<span style="color:blue;">{reaction}</span>'
        elif "Blocked" in reaction:
            colored_entry = f'<span style="color:red;">{reaction}</span>'
        elif "Unblocked" in reaction:
            colored_entry = f'<span style="color:purple;">{reaction}</span>'
        else:
            colored_entry = reaction
        self.event_log.append(colored_entry)
        self.event_log.verticalScrollBar().setValue(self.event_log.verticalScrollBar().maximum())

    def on_algo_changed(self, text):
        self.quantum_spin.setEnabled(text == "Round Robin")
