import ctypes
import os
import platform
import time

# Path setup
current_dir = os.path.dirname(__file__)
bin_dir = os.path.abspath(os.path.join(current_dir, '../bin'))

if platform.system() == "Darwin":
    lib_name = 'libmyscheduler.dylib'
elif platform.system() == "Linux":
    lib_name = 'libmyscheduler.so'
elif platform.system() == "Windows":
    lib_name = 'myscheduler.dll'
else:
    raise Exception(f"Unsupported OS: {platform.system()}")

lib_path = os.path.join(bin_dir, lib_name)
lib = ctypes.CDLL(lib_path)

# Function prototypes
lib.api_init_scheduler.argtypes = [ctypes.c_int, ctypes.c_int]
lib.api_init_scheduler.restype = None

lib.load_process_from_file.argtypes = [ctypes.c_char_p, ctypes.c_int]
lib.load_process_from_file.restype = None

lib.step_execution.restype = None
lib.get_clock_cycle.restype = ctypes.c_int
lib.get_latest_log.restype = ctypes.c_char_p

lib.is_waiting_for_gui_input.restype = ctypes.c_int
lib.set_gui_input.argtypes = [ctypes.c_char_p]
lib.set_gui_input.restype = None

# Initialize scheduler
print("➡️ Initializing Scheduler with MLFQ...")
lib.api_init_scheduler(2, 2)

# Load programs
programs = [
    ("program3.txt", 0),
    ("program1.txt", 1),
    ("program2.txt", 3)
]

base_dir = os.path.abspath(os.path.join(current_dir, '..'))
for fname, arrival in programs:
    path = os.path.join(base_dir, fname)
    print(f"📥 Loading {fname} (Arrival: {arrival})")
    lib.load_process_from_file(ctypes.c_char_p(path.encode()), ctypes.c_int(arrival))

print("✅ All processes loaded.\n")

# Step execution with input handling
last_seen_log = ""
MAX_STEPS = 30  # You can increase this if needed

for i in range(MAX_STEPS): 
    print(f"\n🚀 [STEP {i+1}] Step Execution...")
    lib.step_execution()
    clock = lib.get_clock_cycle()
    log = lib.get_latest_log().decode().strip()

    print(f"🕒 Clock Cycle: {clock}")
    if log and log != last_seen_log:
        print(f"📝 Log: {log}")
        last_seen_log = log
    else:
        print("🟢 No new log.")

    # Auto send input if backend is waiting
    if lib.is_waiting_for_gui_input() == 1:
        user_input = f"TestInput_{i+1}"
        print(f"🖋️ Backend is waiting for input, sending: {user_input}")
        lib.set_gui_input(user_input.encode())

    # Optional: pause between steps (can remove if you want fast execution)
    time.sleep(0.3)

print("\n✅ Scenario complete. Review logs above to verify step-by-step behavior.")