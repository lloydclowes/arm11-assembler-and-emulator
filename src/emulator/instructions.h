#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim

  Instructions module for execution of specific sets of instructions.
*/
#include "processor.h"
#include "utility.h"

#define GPIO_CTRL_1 0x20200000
#define GPIO_CTRL_2 0x20200004
#define GPIO_CTRL_3 0x20200008
#define GPIO_CLEAR 0x20200028
#define GPIO_SET 0x2020001C
#define NUM_GPIO_LOCATIONS 5

void data_processing(struct State *state);
void multiply(struct State *state);
void transfer(struct State *state);
void gpio_transfer(struct State *state);
void branch(struct State *state);

bool is_gpio_access(uint32_t address);
void ouput_gpio_action(uint32_t address);
int gpio_memory_index(uint32_t address);
#endif
