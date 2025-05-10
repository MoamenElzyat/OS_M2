# Generate a full improved README.md with expanded details based on actual project structure

new_readme = """# 🧠 Process Scheduler Simulation

This project is a GUI-based **Operating Systems process scheduler simulator**, developed for academic purposes (Mini Project 2). It supports simulation of major scheduling algorithms with clear visualization, interactivity, and synchronization primitives.

## 🚀 Supported Scheduling Algorithms

- **FCFS** – First Come First Serve
- **RR** – Round Robin (configurable quantum)
- **MLFQ** – Multi-Level Feedback Queue (4 levels)

## 🧩 Key Features

- ✅ Semaphore-based synchronization with mutex locks
- ⛓️ Blocking and unblocking processes on resources
- 🧠 Instruction execution engine with PCB state tracking
- 🔁 Dynamic visualization of ready and blocked queues
- 📟 Memory segment simulation with variable access
- 👁️ Real-time GUI control with step and auto modes

## 🛠️ Tech Stack

- **C** – core logic and simulation engine
- **Python (PyQt5)** – GUI controller (under `gui/`)
- **Makefile** – builds a dynamic C library
- **GCC** – for compiling the C backend

## 📂 Directory Structure

```
├── include/               # Header files for all C components
├── src/                   # Source files implementing scheduler logic
├── gui/                   # Python GUI frontend with controller
├── bin/                   # Compiled dynamic library
├── obj/                   # Object files for compilation
├── program1.txt           # Sample instruction set for process 1
├── program2.txt           # Sample instruction set for process 2
├── program3.txt           # Sample instruction set for process 3
├── Makefile               # Build automation script
├── test.txt               # Console logging or debug test file
├── scheduler_gui.log      # Log file from GUI interactions
```

## 🧪 How to Build and Run

### Prerequisites

Make sure the following are installed:

- `gcc`
- `make`
- `python3`
- `PyQt5` (install via `pip install PyQt5`)

### Build the Simulation Library

```bash
make clean && make build-lib
```

This generates the shared object (`.dylib` or `.so`) under `bin/`.

### Launch the GUI

```bash
python3 gui/scheduler_controller.py
```

The GUI will allow:
- Loading of program files
- Adding processes dynamically
- Executing step-by-step or automatically
- Visualizing queue and memory changes

---

## 👨‍💻 Author

**Moamen Abdelrahman**  
Operating Systems Project 2  
German University in Cairo (GUC)

---

## 📜 License

This project is for **educational use only** as part of the GUC CSEN602 Operating Systems course.
"""

