#ifndef EMULATE_H
#define EMULATE_H

/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim

  Emulate module for main execution of the emulator.
*/

#include "emulator/instructions.h"
#include "emulator/processor.h"
#include "emulator/utility.h"

void fill_pipeline(struct State *state);

uint32_t fetch(struct State *state);
struct Instruction decode(struct State *state);
bool execute(struct State *state);
void print_results(struct State *state);

#endif
