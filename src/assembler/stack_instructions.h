#ifndef ASSEMBLE_STACK_INSTRUCTIONS_H__
#define ASSEMBLE_STACK_INSTRUCTIONS_H__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "instructions.h"

/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim
*/

uint32_t *stack_store(struct Instruction instr, struct Map *symbol_map);
uint32_t *stack_load(struct Instruction instr, struct Map *symbol_map);
uint32_t stack_get_Rlist(char **Rlist, struct Map *symbol_map);
bool is_stack_operation(char *opcode);

#endif
