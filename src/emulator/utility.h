#ifndef EMULATE_UTILITY_H
#define EMULATE_UTILITY_H

/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim

  Utility module for all base operations.
*/
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>

uint32_t signed_offset(uint32_t offset);
bool get_bit_at(uint32_t instruction, uint8_t position);
uint32_t get_between_range(uint32_t instruction, uint8_t from, uint8_t to);
#endif
