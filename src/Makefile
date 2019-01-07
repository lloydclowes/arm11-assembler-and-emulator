#CC      = gcc
CFLAGS  = -Wall -g -D_POSIX_SOURCE -D_BSD_SOURCE -std=c99 -pedantic -Werror

all: assemble emulate

assemble: assemble.o assembler/directive_processor.o assembler/stack_instructions.o assembler/stack_instructions.o assembler/instructions.o assembler/utilities.o assembler/map.o
	$(CC) assemble.o assembler/directive_processor.o assembler/stack_instructions.o assembler/instructions.o assembler/utilities.o assembler/map.o -o assemble

assemble.o: assemble.c assemble.h assembler/directive_processor.h assembler/stack_instructions.h assembler/instructions.h assembler/utilities.h  assembler/map.h
	$(CC) $(CFLAGS) assemble.c -c -o assemble.o

assembler/stack_instructions.o: assembler/stack_instructions.c assembler/stack_instructions.h assembler/instructions.h assembler/utilities.h  assembler/map.h
	$(CC) $(CFLAGS) assembler/stack_instructions.c -c -o assembler/stack_instructions.o

assembler/directive_processor.o: assembler/directive_processor.c assembler/directive_processor.h assembler/instructions.h assembler/utilities.h  assembler/map.h
	$(CC) $(CFLAGS) assembler/directive_processor.c -c -o assembler/directive_processor.o

assembler/instructions.o: assembler/instructions.c assembler/instructions.h assembler/utilities.h  assembler/map.h
	$(CC) $(CFLAGS) assembler/instructions.c -c -o assembler/instructions.o

assembler/utilities.o: assembler/utilities.c assembler/utilities.h assembler/map.h
	$(CC) $(CFLAGS) assembler/utilities.c -c -o assembler/utilities.o

assembler/map.o: assembler/map.c assembler/map.h
	$(CC) $(CFLAGS) assembler/map.c -c -o assembler/map.o

emulate: emulate.o emulator/instructions.o emulator/processor.o emulator/utility.o
	$(CC) emulate.o emulator/instructions.o emulator/processor.o emulator/utility.o -o emulate

emulate.o: emulate.c emulate.h emulator/instructions.h emulator/processor.h emulator/utility.h
	$(CC) $(CFLAGS) emulate.c -c -o emulate.o

emulator/instructions.o: emulator/instructions.c emulator/instructions.h emulator/processor.h emulator/utility.h
	$(CC) $(CFLAGS) emulator/instructions.c -c -o emulator/instructions.o

emulator/processor.o: emulator/processor.c emulator/processor.h emulator/utility.h
	$(CC) $(CFLAGS) emulator/processor.c -c -o emulator/processor.o

emulator/utility.o: emulator/utility.c emulator/utility.h
	$(CC) $(CFLAGS) emulator/utility.c -c -o emulator/utility.o

clean:
	rm -f $(wildcard *.o)
	rm -f $(wildcard assembler/*.o)
	rm -f $(wildcard emulator/*.o)
	rm -f assemble
	rm -f emulate

.SUFFIXES: .c .o

.PHONY: all clean emulate assemble
