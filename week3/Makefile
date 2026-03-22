# Detect OS
ifeq ($(OS),Windows_NT)
    RM = del /f /q
    EXE = .exe
else
    RM = rm -f
    EXE =
endif

# Try system flex, fallback to local flex.exe
FLEX := $(shell command -v flex 2>/dev/null)
ifeq ($(FLEX),)
    FLEX = flex-2.5.4a-1-bin/bin/flex.exe
endif

# Compiler
CC = gcc
CFLAGS = 

# Files
LEX_FILE = a3_230101077.l
LEX_OUT = lex.yy.c
HASH = hash.c
TARGET = lexer$(EXE)

# Default target
all: $(TARGET)

# Generate lexer
$(LEX_OUT): $(LEX_FILE)
	$(FLEX) $(LEX_FILE)

# Compile
$(TARGET): $(LEX_OUT) $(HASH)
	$(CC) $(CFLAGS) $(LEX_OUT) $(HASH) -o $(TARGET)

# Run
run: $(TARGET)
	./$(TARGET) a3_230101077_test.nc

# Clean
clean:
	-$(RM) $(LEX_OUT) $(TARGET)
