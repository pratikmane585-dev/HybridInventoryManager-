# ── Hybrid Inventory Manager – Makefile ──────────────────────────────────────
#
# C files   → compiled with $(CC)  (gcc)
# C++ files → compiled with $(CXX) (g++)
# Final link is done by g++ so the C++ runtime is included automatically.
# ──────────────────────────────────────────────────────────────────────────────

CC      := gcc
CXX     := g++

CFLAGS   := -std=c11   -Wall -Wextra -Iinclude
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude

TARGET  := inventory_manager

# Source files
C_SRCS   := src/inventory.c
CXX_SRCS := src/InventoryManager.cpp src/main.cpp

# Object files (placed in build/)
C_OBJS   := $(patsubst src/%.c,   build/%.o, $(C_SRCS))
CXX_OBJS := $(patsubst src/%.cpp, build/%.o, $(CXX_SRCS))
OBJS     := $(C_OBJS) $(CXX_OBJS)

# ── Default target ────────────────────────────────────────────────────────────
.PHONY: all clean run

all: build $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo ""
	@echo "  Build successful → ./$(TARGET)"

# ── Compile C sources ─────────────────────────────────────────────────────────
build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ── Compile C++ sources ───────────────────────────────────────────────────────
build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ── Utility ───────────────────────────────────────────────────────────────────
build:
	mkdir -p build

run: all
	./$(TARGET)

clean:
	rm -rf build $(TARGET) inventory.dat
	@echo "  Cleaned."
