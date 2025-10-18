CC       := gcc
CFLAGS   := -std=c99 -O2 -I./src \
             -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wcast-qual -Wcast-align \
             -Wpointer-arith -Wstrict-overflow=5 -Wundef -Wwrite-strings -Wlogical-op \
             -Waggregate-return -Wfloat-equal -Winline -Wredundant-decls -Wstrict-prototypes \
             -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wold-style-definition \
             -Wbad-function-cast -Wjump-misses-init -Wuninitialized -Wmaybe-uninitialized \
             -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wformat=2 -Wdouble-promotion \
             -Wvla -Walloc-zero -Walloca -Wstringop-overflow=4 -fanalyzer

SRC_DIR  := src
BUILD_DIR := build
TARGET   := kmas.exe

SOURCES  := $(wildcard $(SRC_DIR)/*.c)
OBJECTS  := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@
	@echo "✔ Build complete: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
    rm vgcore.*
    rm */vgcore.*
	@echo "🧹 Clean complete"

rebuild: clean all

valgrind: $(TARGET)
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes --error-exitcode=1 --track-fds=yes \
	         --trace-children=yes --num-callers=50 ./$(TARGET)

.PHONY: all clean rebuild valgrind
