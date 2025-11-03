# ===== Compiler and Directories =====
CC        := gcc
SRC_DIR   := src
BUILD_DIR := build
TARGET    := kmas.exe

# ===== Common Flags =====
CFLAGS_COMMON := -std=c99 -O2 -I./src \
  -Wall -Wextra -Wpedantic -Wconversion \
  -Wshadow -Wshadow=compatible-local -Wcast-qual -Wcast-align \
  -Wpointer-arith -Wstrict-overflow=5 -Wundef -Wwrite-strings -Wlogical-op \
  -Waggregate-return -Wfloat-equal -Winline -Wredundant-decls -Wstrict-prototypes \
  -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wold-style-definition \
  -Wbad-function-cast -Wjump-misses-init -Wuninitialized -Wmaybe-uninitialized \
  -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wformat=2 -Wdouble-promotion \
  -Wvla -Walloc-zero -Walloca -Wstringop-overflow=4 -fanalyzer \
  -fstack-protector-all -Wformat-security -Wfatal-errors \
  -Wstrict-aliasing=2 -Wimplicit-fallthrough -Wnonnull \
  -Wduplicated-cond -Wduplicated-branches -Wunreachable-code -Wno-nonnull

# ===== Sanitizer Flags =====
SANFLAGS := -fsanitize=address,undefined,float-divide-by-zero,null \
            -fno-omit-frame-pointer -g -O1 -fstack-protector-strong \
            -fPIE -pie -Wl,-z,relro,-z,now

ASAN_OPTIONS := detect_leaks=1:halt_on_error=1:abort_on_error=1:strict_string_checks=1:\
detect_stack_use_after_return=1:detect_invalid_pointer_pairs=1:use_odr_indicator=1:\
check_initialization_order=1

UBSAN_OPTIONS := print_stacktrace=1:halt_on_error=1:print_type_mismatch=1:print_alignment=1

# ===== Build Mode (default: normal) =====
CFLAGS   := $(CFLAGS_COMMON)
LDFLAGS  :=

# ===== Sources and Objects =====
SOURCES  := $(wildcard $(SRC_DIR)/*.c)
OBJECTS  := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

# ===== Default target =====
all: clean $(TARGET)

# ===== Normal Build =====
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "✔ Build complete: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

# ===== ASAN Build =====
asan: CFLAGS += $(SANFLAGS)
asan: LDFLAGS += $(SANFLAGS)
asan: clean $(TARGET)
	@echo "🧠 ASAN build complete: $(TARGET)"
	@echo "Run with: make run-asan"

# ===== ASAN Build + Run =====
run-asan: CFLAGS += $(SANFLAGS)
run-asan: LDFLAGS += $(SANFLAGS)
run-asan: clean $(TARGET)
	@echo "🚀 Running ASAN-instrumented binary..."
	ASAN_OPTIONS="$(ASAN_OPTIONS)" \
	UBSAN_OPTIONS="$(UBSAN_OPTIONS)" \
	./$(TARGET) source.kas -i -v

# ===== Valgrind Build + Run =====
valgrind: clean
	@echo "🔍 Building without sanitizers for Valgrind..."
	$(MAKE) --no-print-directory all CFLAGS="$(CFLAGS_COMMON)" LDFLAGS=""
	@echo "Running under Valgrind..."
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes --error-exitcode=1 --track-fds=yes \
	         --trace-children=yes --num-callers=50 ./$(TARGET) source.kas -i -v

# ===== Utilities =====
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "🧹 Clean complete"

rebuild: clean all

.PHONY: all asan run-asan valgrind clean rebuild
