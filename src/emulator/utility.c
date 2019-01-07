#include "utility.h"

/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim
*/

/*
  Returns bool value at bit position in instruction.
*/
bool get_bit_at(uint32_t instruction, uint8_t position) {
  instruction >>= position;
  return instruction % 2;
}

/*
  Returns the signed offset for executing branch instructions.
*/
uint32_t signed_offset(uint32_t offset) {
  uint32_t bit_mask = ~(0xC000000);
  uint32_t sign_mask = 0x2000000;
  offset = offset & bit_mask;
  if (offset & sign_mask) {
    for (int i = 26; i < 32; i++) {
      offset |= (1 << i);
    }
  }
  return offset;
}

/*
  Returns uint32_t between range inclusive.
*/
uint32_t get_between_range(uint32_t instruction, uint8_t from, uint8_t to) {
  assert(to < 32);
  return (instruction >> from) & ~(~0 << (to - from + 1));
}
