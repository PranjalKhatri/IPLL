CC      = gcc
CFLAGS  = -Wall -Wextra -g
TARGET  = assembler

SRCS    = $(wildcard *.c)
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	- rm -f $(TARGET) $(OBJS) *.exe
	- del $(TARGET) $(OBJS) *.exe
