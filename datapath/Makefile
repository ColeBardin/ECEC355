SOURCE	:= main.c parser.c instruction.c registers.c core.c
CC	:= gcc
CCFLAGS := -std=gnu99
TARGET	:= assembler

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) -o $(TARGET) $(SOURCE) $(CCFLAGS)

clean:
	rm -f $(TARGET)
