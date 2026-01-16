ASM_SRC := $(wildcard src/*.asm)
C_SRC   := $(wildcard src/*.c)
CPP_SRC := $(wildcard src/*.cpp)

OBJ_DIR := obj
BIN_DIR := bin

# Logic to create object file list remains solid
OBJ := $(ASM_SRC:src/%.asm=$(OBJ_DIR)/asm_%.o) \
       $(C_SRC:src/%.c=$(OBJ_DIR)/c_%.o) \
       $(CPP_SRC:src/%.cpp=$(OBJ_DIR)/cpp_%.o)

EXE := $(BIN_DIR)/main

NASM := nasm
NASM_FLAGS := -f elf32 -g -Isrc/

CC  := gcc
CXX := g++
# Use -m32 to match your elf32 NASM output
COMMON_FLAGS := -m32 -g -Isrc
CFLAGS   := $(COMMON_FLAGS)
CXXFLAGS := $(COMMON_FLAGS) -std=c++23

# Use g++ for linking when C++ files are involved to link the STL correctly
LD := g++
LDFLAGS := -m32 -g

all: setup $(EXE)

setup:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

$(EXE): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# NASM Rule
$(OBJ_DIR)/asm_%.o: src/%.asm
	$(NASM) $(NASM_FLAGS) $< -o $@

# C Rule
$(OBJ_DIR)/c_%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# C++ Rule
$(OBJ_DIR)/cpp_%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean setup
