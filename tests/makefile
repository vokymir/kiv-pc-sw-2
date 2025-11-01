# ---------------------------
#  Valgrind-driven test Makefile
#  Location: tests/makefile
# ---------------------------

# Compiler setup
CC       := gcc
CFLAGS   := -std=c99 -O2 -I../src \
             -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wcast-qual -Wcast-align \
             -Wpointer-arith -Wstrict-overflow=5 -Wundef -Wwrite-strings -Wlogical-op \
             -Waggregate-return -Wfloat-equal -Winline -Wredundant-decls -Wstrict-prototypes \
             -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wold-style-definition \
             -Wbad-function-cast -Wjump-misses-init -Wuninitialized -Wmaybe-uninitialized \
             -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wformat=2 -Wdouble-promotion \
             -Wvla -Walloc-zero -Walloca -Wstringop-overflow=4 -fanalyzer

VALGRIND := valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
             --track-origins=yes --error-exitcode=1 --track-fds=yes \
             --trace-children=yes --num-callers=50

SRC_DIR   := ../src
BUILD_DIR := ../build
BIN_DIR   := ../bin
TEST_DIR  := .

# Discover all test source files
TEST_SRCS := $(wildcard $(TEST_DIR)/test_*.c)
TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/%,$(TEST_SRCS))

# Project sources (exclude main.c!)
SRC_FILES := $(filter-out $(SRC_DIR)/main.c, $(wildcard $(SRC_DIR)/*.c))
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

# --------------------------------------------
# Default target
# --------------------------------------------
.PHONY: test all clean
test: all

# Build + run all tests
all: $(TEST_BINS)
	@echo "🧪 Running all tests under Valgrind..."
	@for t in $(TEST_BINS); do \
		echo "▶ Running $$t..."; \
		$(VALGRIND) $$t || { echo "❌ Test $$t failed under Valgrind"; exit 1; }; \
		echo "✅ $$t passed."; \
	done
	@echo "🎉 All tests passed without leaks or errors!"

# --------------------------------------------
#  Build rules
# --------------------------------------------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "⚙️  Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/test_%.o: $(TEST_DIR)/test_%.c | $(BUILD_DIR)
	@echo "⚙️  Compiling test $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/test_%: $(BUILD_DIR)/test_%.o $(OBJ_FILES) | $(BIN_DIR)
	@echo "🔗 Linking $@..."
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# --------------------------------------------
# Cleanup
# --------------------------------------------
clean:
	@echo "🧹 Cleaning build and bin directories..."
	rm vgcore.*
	rm -rf $(BUILD_DIR) $(BIN_DIR)

