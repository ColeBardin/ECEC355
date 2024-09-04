VERBOSE ?= 0
SOURCE	:= main.c parser.c instruction.c registers.c core.c
CC	:= gcc
CCFLAGS := -std=gnu99
CCFLAGS += -DVERBOSE=$(VERBOSE)
TARGET	:= RISCV_core

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) -o $(TARGET) $(SOURCE) $(CCFLAGS)

clean:
	rm -f $(TARGET)
