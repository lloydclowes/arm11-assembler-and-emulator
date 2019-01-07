#ifndef PROCESSOR_H
#define PROCESSOR_H

/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim

  Processor module used to store instruction types, state and system update
  functions.
*/

#include "utility.h"

#define CAPACITY 1 << 16

#define LR 14
#define PC 15
#define CPSR 16
#define N 0
#define Z 1
#define C 2
#define V 3


enum InstrType { DATA_PROCESS, MULT, TRANSFER, BRANCH, HALT };

struct Instruction {
  enum InstrType instr;
  uint8_t cond;
  bool I;
  bool A;
  bool S;
  bool P;
  bool U;
  bool L;
  bool temp_c;
  uint8_t Rn;
  uint8_t Rd;
  uint8_t Rs;
  uint8_t Rm;
  uint8_t op_code;
  uint32_t operand2;
  uint32_t offset;
  uint32_t raw_value;
};

struct State {
  uint8_t *memory;
  uint32_t *gpio_memory;
  uint32_t *registers;
  bool running;

  /* Pipeline */
  uint32_t fetched;
  struct Instruction decoded;
};

uint32_t read_memory(struct State *state, int counter);
uint32_t read_offset(struct State *state, uint8_t register_index, int counter);
uint32_t perform_shift(struct State *state, bool isfetch, int *carry);
void update_CPSR(uint8_t n, uint8_t z, uint8_t c, uint8_t v,
                 struct State *state);
void get_flags(struct State *state, uint8_t *flags);
void set_flags(uint32_t result, struct State *state);
bool check_condition(struct State *state);

#endif
