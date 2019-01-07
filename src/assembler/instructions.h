#ifndef ASSEMBLE_INSTRUCTIONS_H__
#define ASSEMBLE_INSTRUCTIONS_H__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "utilities.h"

/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim
*/

enum InstrType {
  DATA_PROCESSING, MULTIPLY, SINGLE_DATA_TRANSFER, BRANCH, STACK_LOAD, STACK_STORE, SHIFT,
  DIRECTIVE
};

struct Instruction {
  char *opcode;
  enum InstrType instr_type;
  uint8_t cond;

  char *stack_mode;
  char **Rlist;

  char *Rd;
  char *Rn;
  char *operand2;

  char *Rm;
  char *Rs;

  char *address;

  char *expression;

  bool has_address;
  uint16_t memory_address;

  char **directive_arguments;
};

uint32_t *branch(struct Instruction *instr, struct Map *symbol_map);
uint32_t *transfer(struct Instruction *instr, struct Map *symbol_map);
uint32_t *multiply(struct Instruction instr, struct Map *symbol_map);
uint32_t *data_process(struct Instruction instr, struct Map *symbol_map);
uint32_t *shift(struct Instruction instr, struct Map *symbol_map);
bool check_if_opcode(char *opcode);
enum InstrType get_instr_type(char *opcode);
int32_t calculate_offset(uint32_t PC, uint32_t value, struct Map *symbol_map);
int32_t decode_expression(char *expr, struct Map *symbol_map);

#endif
