# Executable
BINFOLDER := bin/

# Include files
INCFOLDER := inc/

# Source files
SRCFOLDER := src/

# Object files
OBJFOLDER := obj/

CC := gcc
CFLAGS := -W -Wall
SRCFILES := $(wildcard src/*.c)
all: $(SRCFILES:src/%.c=obj/%.o)
	$(CC) $(CFLAGS) obj/*.o -o bin/bin
obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I./inc
run: bin/bin
	bin/bin
.PHONY: clean
clean:
	rm -rf obj/*
	rm -rf bin/*
