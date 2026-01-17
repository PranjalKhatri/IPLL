MODE ?= main

OBJ_DIR := obj/$(MODE)
BIN_DIR := bin

SRC_DIR  := src
TEST_DIR := test

SRC_ASM := $(wildcard $(SRC_DIR)/*.asm)
SRC_C   := $(wildcard $(SRC_DIR)/*.c)
SRC_CPP := $(wildcard $(SRC_DIR)/*.cpp)

TEST_ASM := $(wildcard $(TEST_DIR)/*.asm)
TEST_C   := $(wildcard $(TEST_DIR)/*.c)
TEST_CPP := $(wildcard $(TEST_DIR)/*.cpp)

ifeq ($(MODE),test)
SRC_ASM := $(filter-out $(SRC_DIR)/main%.asm,$(SRC_ASM))
SRC_C   := $(filter-out $(SRC_DIR)/main%.c,$(SRC_C))
SRC_CPP := $(filter-out $(SRC_DIR)/main%.cpp,$(SRC_CPP))

ASM_SRC := $(SRC_ASM) $(TEST_ASM)
C_SRC   := $(SRC_C)   $(TEST_C)
CPP_SRC := $(SRC_CPP) $(TEST_CPP)

EXE := $(BIN_DIR)/test
else
ASM_SRC := $(SRC_ASM)
C_SRC   := $(SRC_C)
CPP_SRC := $(SRC_CPP)

EXE := $(BIN_DIR)/main
endif

OBJ := $(ASM_SRC:%.asm=$(OBJ_DIR)/asm_%.o) \
       $(C_SRC:%.c=$(OBJ_DIR)/c_%.o) \
       $(CPP_SRC:%.cpp=$(OBJ_DIR)/cpp_%.o)

NASM := nasm
NASM_FLAGS := -f elf32 -g -Isrc -Itest

CC  := gcc
CXX := g++

COMMON_FLAGS := -m32 -g -Isrc -Itest
CFLAGS   := $(COMMON_FLAGS)
CXXFLAGS := $(COMMON_FLAGS) -std=c++23

LD := g++
LDFLAGS := -m32 -g

all: setup $(EXE)

test:
	$(MAKE) MODE=test

setup:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

$(EXE): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/asm_%.o: %.asm
	@mkdir -p $(dir $@)
	$(NASM) $(NASM_FLAGS) $< -o $@

$(OBJ_DIR)/c_%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/cpp_%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj bin

.PHONY: all test clean setup
