#include "instructions.h"

/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim
*/

/*
  Executes the set of branch instructions given the state.
*/
void branch(struct State *state) {
  state->registers[PC] += signed_offset(state->decoded.offset);
}

/*
  Executes the set of single data transfer instructions given the state.
*/
void transfer(struct State *state) {
  int sign_bit = state->decoded.U ? 1 : -1;
  uint8_t rn = state->decoded.Rn;
  uint8_t rd = state->decoded.Rd;
  uint8_t rm = state->decoded.Rm;
  uint32_t rn_value = state->registers[rn];
  uint32_t rm_value = state->registers[rm];
  uint32_t new_address = rn_value;

  uint32_t offset_value = 0;
  if (state->decoded.I) {
    offset_value = perform_shift(state, false, NULL);
  } else {
    offset_value = state->decoded.offset;
  }

  if(~state->decoded.P && new_address >= CAPACITY && is_gpio_access(new_address)){
    /* Post index */
    ouput_gpio_action(new_address + (offset_value * sign_bit));
    gpio_transfer(state);
    return;
  }else if(
    state->decoded.P == 1
    && (new_address + (offset_value * sign_bit) >= CAPACITY)
    && (is_gpio_access(new_address + (offset_value * sign_bit)))
    ){
    /* Pre index */
    ouput_gpio_action(new_address + (offset_value * sign_bit));
    gpio_transfer(state);
    return;
  }else if (new_address + (offset_value * sign_bit) >= CAPACITY) {
    printf("Error: Out of bounds memory access at address 0x%08x\n",
          new_address + (offset_value * sign_bit));
          return;
  }

  if (state->decoded.P) {
    new_address = rn_value + (offset_value * sign_bit);
    if (state->decoded.L) {
      state->registers[rd] = read_memory(state, new_address);
    } else {
      for (int i = 0; i < 4; i++) {
        state->memory[new_address + i] =
            get_between_range(state->registers[rd], i * 8, ((i + 1) * 8) - 1);
      }
    }
  } else {
    if (rn_value == rm_value) {
      return;
    }
    if (state->decoded.L) {
      state->registers[rd] = read_memory(state, rn_value);
      state->registers[rn] = read_offset(state, rn, offset_value);
    } else {
      for (int i = 0; i < 4; i++) {
        state->memory[new_address + i] =
            get_between_range(state->registers[rd], i * 8, ((i + 1) * 8) - 1);
      }
      new_address = rn_value + (offset_value * sign_bit);
      state->registers[rn] = new_address;
    }
  }
}

void gpio_transfer(struct State *state) {
  int sign_bit = state->decoded.U ? 1 : -1;
  uint8_t rn = state->decoded.Rn;
  uint8_t rd = state->decoded.Rd;
  uint8_t rm = state->decoded.Rm;
  uint32_t rn_value = state->registers[rn];
  uint32_t rm_value = state->registers[rm];
  uint32_t new_address = rn_value;

  uint32_t offset_value = 0;
  if (state->decoded.I) {
    offset_value = perform_shift(state, false, NULL);
  } else {
    offset_value = state->decoded.offset;
  }

  if (state->decoded.P) {
    new_address = rn_value + (offset_value * sign_bit);
    if (state->decoded.L) {
      state->registers[rd] = new_address;
    } else {
      state->gpio_memory[gpio_memory_index(new_address)] = (state->registers[rd]);
    }
  } else {
    if (rn_value == rm_value) {
      return;
    }
    if (state->decoded.L) {
      state->registers[rd] = state->memory[rn_value];
      state->registers[rn] = state->memory[new_address];
    } else {
      state->gpio_memory[gpio_memory_index(new_address)] = state->registers[rd];
      new_address = rn_value + (offset_value * sign_bit);
      state->registers[rn] = new_address;
    }
  }
}

/*
  Executes the set of multiply instructions given the state.
*/
void multiply(struct State *state) {

  uint32_t Rd = state->decoded.Rd;
  uint32_t Rn = state->decoded.Rn;
  uint32_t Rs = state->decoded.Rs;
  uint32_t Rm = state->decoded.Rm;
  uint32_t A = state->decoded.A;
  uint32_t S = state->decoded.S;

  uint32_t res = state->registers[Rm] * state->registers[Rs];

  if (A) {
    res += state->registers[Rn];
  }
  if (S) {
    uint8_t flags[4] = {0, 0, 0, 0};
    get_flags(state, flags);
    uint32_t n = res < 0 ? 1 : 0;
    uint32_t z = res == 0 ? 1 : 0;
    update_CPSR(n, z, flags[C], flags[V], state);
  }
  state->registers[Rd] = res;
}

/*
  Executes the set of data procesing instructions.
*/
void data_processing(struct State *state) {

  uint32_t op_code = state->decoded.op_code;
  uint8_t S = state->decoded.S;
  uint32_t Rn = state->decoded.Rn;
  uint32_t Rd = state->decoded.Rd;
  uint32_t operand2 = state->decoded.operand2;

  assert(Rn < 15);
  assert(Rd < 15);

  uint32_t result;

  switch (op_code) {
  case 0:
    result = state->registers[Rn] & operand2;
    state->registers[Rd] = result;
    break;
  case 1:
    result = state->registers[Rn] ^ operand2;
    state->registers[Rd] = result;
    break;
  case 2:
    result = state->registers[Rn] - operand2;
    state->registers[Rd] = result;
    state->decoded.temp_c = !(operand2 > state->registers[Rn]);
    break;
  case 3:
    result = operand2 - state->registers[Rn];
    state->registers[Rd] = result;
    state->decoded.temp_c = !(operand2 < state->registers[Rn]);
    break;
  case 4:
    result = state->registers[Rn] + operand2;
    state->registers[Rd] = result;
    state->decoded.temp_c =
        (result < state->registers[Rn]) || (result < operand2);
    break;
  case 8:
    result = state->registers[Rn] & operand2;
    break;
  case 9:
    result = state->registers[Rn] ^ operand2;
    break;
  case 10:
    result = state->registers[Rn] - operand2;
    state->decoded.temp_c = !(operand2 > state->registers[Rn]);
    break;
  case 12:
    result = state->registers[Rn] | operand2;
    state->registers[Rd] = result;
    break;
  case 13:
    result = operand2;
    state->registers[Rd] = result;
    break;
  }

  if (S) {
    set_flags(result, state);
  }
}

/*
  Returns true iff address corresponds to the address of GPIO pints
*/
bool is_gpio_access(uint32_t address){
  return address == GPIO_CTRL_1 || address == GPIO_CTRL_2
         || address == GPIO_CTRL_3 || address == GPIO_SET
         || address == GPIO_CLEAR;
}

void ouput_gpio_action(uint32_t address){
  assert(is_gpio_access(address));
  switch(address){
    case GPIO_CTRL_1:
      //new_address + (offset_value * sign_bit)
      printf("One GPIO pin from 0 to 9 has been accessed\n");
      break;
    case GPIO_CTRL_2:
      //new_address + (offset_value * sign_bit)
      printf("One GPIO pin from 10 to 19 has been accessed\n");
      break;
    case GPIO_CTRL_3:
      //new_address + (offset_value * sign_bit)
      printf("One GPIO pin from 20 to 29 has been accessed\n");
      break;
    case GPIO_CLEAR:
      //new_address + (offset_value * sign_bit)
      printf("PIN OFF\n");
      break;
    case GPIO_SET:
      //new_address + (offset_value * sign_bit)
      printf("PIN ON\n");
      break;
  }
}

int gpio_memory_index(uint32_t address){
  assert(is_gpio_access(address));
  switch(address){
    case GPIO_CTRL_1:
      return 0;
      break;
    case GPIO_CTRL_2:
      return 1;
      break;
    case GPIO_CTRL_3:
      return 2;
      break;
    case GPIO_CLEAR:
      return 3;
      break;
    case GPIO_SET:
      return 4;
      break;
  }
  return 0;
}
