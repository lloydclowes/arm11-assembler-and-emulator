#ifndef ASSEMBLE_H__
#define ASSEMBLE_H__

#include "assembler/map.h"
#include "assembler/utilities.h"
#include "assembler/instructions.h"
#include "assembler/stack_instructions.h"
#include "assembler/directive_processor.h"

/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim
*/

#define RESERVED_FOR_ASSEMBLER 8

uint32_t *to_binary(struct Instruction *instr, struct Map *symbol_map, uint32_t memory_address);
struct Instruction *decode(char **line);
struct Instruction split_opcode(struct Instruction instr, char *opcode);
void free_instruction(struct Instruction instr);
#endif
