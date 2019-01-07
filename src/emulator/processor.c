#include "processor.h"

/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim
*/

/*
  Returns the 32-bit value from memory location.
*/
uint32_t read_memory(struct State *state, int counter) {
  uint8_t *base = state->memory + counter;

  uint32_t ret =
      *base | *(base + 1) << 8 | *(base + 2) << 16 | *(base + 3) << 24;

  return ret;
}

uint32_t read_offset(struct State *state, uint8_t register_index, int counter) {
  return state->registers[register_index] + counter;
}

/*
  Performs a variety of shifts depending on operating mode of the system.

  is_fetch - Whether the shift should be carried out on the fetched instruction
  *carry - Pointer to an int that should be updated with the carry;

  shiftType possible values:
  00 - Logical left (lsl)
  01 - Logical right (lsr)
  10 - Arithmetic right (asr)
  11 - Rotate right (ror)
*/
uint32_t perform_shift(struct State *state, bool is_fetch, int *carry) {

  uint32_t instruction = is_fetch ? state->fetched : state->decoded.raw_value;
  int bit_4 = get_bit_at(instruction, 4);

  uint32_t rm;
  uint32_t rm_value;
  uint32_t shift_type = get_between_range(instruction, 5, 6);

  uint32_t rs;
  uint32_t shift_amount;

  if (is_fetch) {
    rm = get_between_range(instruction, 0, 3);
  } else {
    rm = state->decoded.Rm;
  }

  rm_value = state->registers[rm];

  if (bit_4) {
    rs = get_between_range(instruction, 8, 11);
    shift_amount = get_between_range(state->registers[rs], 0, 8);
  } else {
    shift_amount = get_between_range(instruction, 7, 11);
  }

  uint32_t result = 0;

  int y, z;
  switch (shift_type) {
  case 0: /* lsl */
    result = rm_value << shift_amount;
    break;
  case 1: /* lsr */
    result = rm_value >> shift_amount;
    break;
  case 2: /* asr */
    result = ((signed int)rm_value) >> shift_amount;
    break;
  case 3:
    y = (int)((unsigned)rm_value >> shift_amount);
    z = (int)((unsigned)rm_value << (32 - shift_amount));
    result = y | z;
    break;
  }

  if (carry != NULL) {
    switch (shift_type) {
    case 0: /* lsl */
      *carry = get_bit_at(rm_value, 32 - shift_amount);
      break;
    case 1: /* lsr */
    case 2: /* asr */
    case 3:
      *carry = get_bit_at(rm_value, shift_amount - 1);
      break;
    }
    if (shift_amount == 0) {
      *carry = 0;
    }
  }
  return result;
}

/*
  Provided with a state and a pointer to a uint8_t array of size 4, updates the
  array with the latest flag values from the CSPR.
*/
void get_flags(struct State *state, uint8_t *flags) {
  uint32_t cpsr = state->registers[CPSR];
  for (int i = 0; i < 4; i++) {
    flags[i] = (uint8_t)get_bit_at(cpsr, 31 - i);
  }
}

/*
  Updates the CSPR with the latest flag values, takes in flags n, z, c, and v
  respectively.
*/
void update_CPSR(uint8_t n, uint8_t z, uint8_t c, uint8_t v,
                 struct State *state) {
  uint32_t res = (n << 3) + (z << 2) + (c << 1) + v;
  state->registers[CPSR] = res << 28;
}

/*
  Sets the flags in the CSPR provided with the result from an operation and the
  latest state.
*/
void set_flags(uint32_t result, struct State *state) {
  uint8_t flags[4] = {0, 0, 0, 0};
  get_flags(state, flags);

  int v = flags[V];

  bool z = 0;
  if (!result) {
    z = 1;
  }
  bool n = get_bit_at(result, 31);
  update_CPSR(n, z, state->decoded.temp_c, v, state);
}

/*
  Ensures that the condition flags in the CSPR match the flags required by the
  instruction that is about to be executed.
*/
bool check_condition(struct State *state) {
  uint8_t flags[4] = {0, 0, 0, 0};
  get_flags(state, flags);

  switch (state->decoded.cond) {
  case 0: /* eq */
    return flags[Z];
  case 1: /* ne */
    return !flags[Z];
  case 10: /* ge */
    return flags[N] == flags[V];
  case 11: /* lt */
    return flags[N] != flags[V];
  case 12: /* gt */
    return !flags[Z] && (flags[N] == flags[V]);
  case 13: /* le */
    return flags[Z] || (flags[N] != flags[V]);
  case 14:
    return true;
  }
  return true;
}
