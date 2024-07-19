SOURCE	:= main.c parser.c registers.c
CC	:= gcc
CCFLAGS := -std=gnu99
TARGET	:= assembler

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) -o $(TARGET) $(SOURCE) $(CCFLAGS)

clean:
	rm -f $(TARGET)
