CC = gcc
CFLAGS = -Wall -Wextra -g `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`

SRC = src/
OBJ = obj/
BIN = bin/
TESTS = tests/
INCLUDE = include/

TARGET = $(BIN)run

SRCS = $(filter-out $(SRC)gui.c, $(wildcard $(SRC)*.c))
OBJS = $(patsubst $(SRC)%.c, $(OBJ)%.o, $(SRCS))

# Mutex Test
TEST_MUTEX = test_mutex
TEST_MUTEX_SRC = $(TESTS)$(TEST_MUTEX).c
TEST_MUTEX_OBJ = $(OBJ)$(TEST_MUTEX).o
TEST_MUTEX_BIN = $(BIN)$(TEST_MUTEX)

# Scheduler Test
TEST_SCHED = test_scheduler
TEST_SCHED_SRC = $(TESTS)$(TEST_SCHED).c
TEST_SCHED_OBJ = $(OBJ)$(TEST_SCHED).o
TEST_SCHED_BIN = $(BIN)$(TEST_SCHED)

# Memory Test
TEST_MEMORY = test_memory
TEST_MEMORY_SRC = $(TESTS)$(TEST_MEMORY).c
TEST_MEMORY_OBJ = $(OBJ)$(TEST_MEMORY).o
TEST_MEMORY_BIN = $(BIN)$(TEST_MEMORY)

# Interpreter Test
TEST_INTERP = test_interpreter
TEST_INTERP_SRC = $(TESTS)$(TEST_INTERP).c
TEST_INTERP_OBJ = $(OBJ)$(TEST_INTERP).o
TEST_INTERP_BIN = $(BIN)$(TEST_INTERP)

all: directories $(TARGET)

directories:
	mkdir -p $(OBJ) $(BIN)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -I$(INCLUDE) -o $(TARGET) $(LDFLAGS)

$(OBJ)%.o: $(SRC)%.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -c $< -o $@

# Mutex Test
$(TEST_MUTEX_BIN): $(TEST_MUTEX_OBJ)
	$(CC) $(TEST_MUTEX_OBJ) src/mutex.c src/pcb.c -I$(INCLUDE) -o $(TEST_MUTEX_BIN)

$(TEST_MUTEX_OBJ): $(TEST_MUTEX_SRC)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c $(TEST_MUTEX_SRC) -o $(TEST_MUTEX_OBJ)

# Scheduler Test
$(TEST_SCHED_BIN): $(TEST_SCHED_OBJ)
	$(CC) $(TEST_SCHED_OBJ) src/scheduler.c src/pcb.c src/queue.c -I$(INCLUDE) -o $(TEST_SCHED_BIN)

$(TEST_SCHED_OBJ): $(TEST_SCHED_SRC)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c $(TEST_SCHED_SRC) -o $(TEST_SCHED_OBJ)

# Memory Test
$(TEST_MEMORY_BIN): $(TEST_MEMORY_OBJ)
	$(CC) $(TEST_MEMORY_OBJ) src/memory.c src/pcb.c -I$(INCLUDE) -o $(TEST_MEMORY_BIN)

$(TEST_MEMORY_OBJ): $(TEST_MEMORY_SRC)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c $(TEST_MEMORY_SRC) -o $(TEST_MEMORY_OBJ)

# Interpreter Test
$(TEST_INTERP_BIN): $(TEST_INTERP_OBJ)
	$(CC) $(TEST_INTERP_OBJ) src/interpreter.c src/mutex.c src/pcb.c src/memory.c -I$(INCLUDE) -o $(TEST_INTERP_BIN)

$(TEST_INTERP_OBJ): $(TEST_INTERP_SRC)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c $(TEST_INTERP_SRC) -o $(TEST_INTERP_OBJ)

# Build shared library for Python GUI (Cross-platform)
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    LIB_EXT = dylib
    LIB_FLAGS = -dynamiclib
else ifeq ($(UNAME_S),Linux)
    LIB_EXT = so
    LIB_FLAGS = -shared -fPIC
else ifeq ($(OS),Windows_NT)
    LIB_EXT = dll
    LIB_FLAGS = -shared
else
    $(error Unsupported OS)
endif

LIB_NAME = libmyscheduler.$(LIB_EXT)
LIB_SRCS = \
    src/globals.c \
    src/scheduler_api.c \
    src/scheduler.c \
    src/pcb.c \
    src/memory.c \
    src/mutex.c \
    src/logger.c \
    src/queue.c \
    src/interpreter.c

build-lib: directories
	$(CC) $(LIB_FLAGS) $(LIB_SRCS) -Iinclude -o bin/$(LIB_NAME)

# Run All Tests
test-all: $(TEST_MUTEX_BIN) $(TEST_SCHED_BIN) $(TEST_MEMORY_BIN) $(TEST_INTERP_BIN)
	@echo "================ Run Mutex Test ================"
	./$(TEST_MUTEX_BIN)
	@echo "================ Run Scheduler Test ================"
	./$(TEST_SCHED_BIN)
	@echo "================ Run Memory Test ================"
	./$(TEST_MEMORY_BIN)
	@echo "================ Run Interpreter Test ================"
	./$(TEST_INTERP_BIN)

# Run individual tests
run-test: $(TEST_MUTEX_BIN)
	./$(TEST_MUTEX_BIN)

run-sched-test: $(TEST_SCHED_BIN)
	./$(TEST_SCHED_BIN)

run-mem-test: $(TEST_MEMORY_BIN)
	./$(TEST_MEMORY_BIN)

run-interp-test: $(TEST_INTERP_BIN)
	./$(TEST_INTERP_BIN)

clean:
	rm -rf $(OBJ) $(BIN)

.PHONY: all clean directories build-lib test-all run-test run-sched-test run-mem-test run-interp-test