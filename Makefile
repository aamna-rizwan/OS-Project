# Makefile - Producer-Consumer project
# Targets:
#   make         build the executable
#   make run     build then run
#   make clean   remove the executable

CC       = gcc
CFLAGS   = -Wall -Wextra -O2
LDFLAGS  = -lpthread
TARGET   = pc
SOURCE   = pc.c

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: run clean
