#include "emulate.h"

/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim

  Emulates the runtime of a version of ARM processor.
*/
int main(int argc, char **argv) {

  /* Reserve 64KB of memory */
  uint32_t *gpio_memory = calloc(NUM_GPIO_LOCATIONS, sizeof(uint32_t));
  uint8_t *memory = calloc(CAPACITY, sizeof(uint8_t));
  uint8_t *memory_index = memory;
  struct State state;

  /* Setup GP registers */
  state.registers = calloc(17, sizeof(uint32_t));
  state.memory = memory;
  state.gpio_memory = gpio_memory;
  state.running = true;

  /* Reads binary code file and stores it to memory */
  FILE *file = fopen(argv[1], "rb");
  assert(file != NULL);

  while (fread(memory_index, sizeof(uint8_t), 4, file) == 4) {
    memory_index += 4;
  }

  fclose(file);

  /* Fill in initial values of fetched and decoded */
  fill_pipeline(&state);

  /* Carry out pipeline until instruction is all 0 */
  while (state.running) {
    if (execute(&state)) {
      if (state.running) {
        state.decoded = decode(&state);
        state.fetched = fetch(&state);
      }
    } else {
      if (state.running) {
        fill_pipeline(&state);
      }
    }
  }

  print_results(&state);

  free(memory);
  free(state.registers);
  free(gpio_memory);


  return EXIT_SUCCESS;
}

/*
  Updates state struct with new fetched/decoded values.
*/
void fill_pipeline(struct State *state) {
  state->fetched = fetch(state);
  state->decoded = decode(state);
  state->fetched = fetch(state);
}

/*
  Returns the next instruction that is to be decoded.
*/
uint32_t fetch(struct State *state) {
  uint32_t memory_value = read_memory(state, state->registers[PC]);
  state->registers[PC] += 4;
  return memory_value;
}

/*
  Parses an instruction into type Instruction struct, loads instruction specific
  values into the struct.
*/
struct Instruction decode(struct State *state) {
  struct Instruction instruction;
  uint32_t rotate, imm;
  uint32_t fetched = state->fetched;

  int *carry = malloc(1 * sizeof(int));

  if (fetched == 0) {
    instruction.instr = HALT;
  } else if (get_bit_at(fetched, 27)) {
    instruction.instr = BRANCH;
    instruction.offset = get_between_range(fetched, 0, 23) << 2;
  } else if (get_bit_at(fetched, 26)) {
    instruction.instr = TRANSFER;
    instruction.cond = get_between_range(fetched, 28, 31);
    instruction.offset = get_between_range(fetched, 0, 11);
    instruction.Rm = get_between_range(fetched, 0, 3);
    instruction.Rn = get_between_range(fetched, 16, 19);
    instruction.Rd = get_between_range(fetched, 12, 15);
    instruction.I = get_bit_at(fetched, 25);
    instruction.P = get_bit_at(fetched, 24);
    instruction.U = get_bit_at(fetched, 23);
    instruction.L = get_bit_at(fetched, 20);
  } else if (get_between_range(fetched, 4, 7) == 9) {
    instruction.instr = MULT;
    instruction.op_code = get_between_range(fetched, 21, 24);
    instruction.Rd = get_between_range(fetched, 16, 19);
    instruction.Rn = get_between_range(fetched, 12, 15);
    instruction.Rs = get_between_range(fetched, 8, 11);
    instruction.Rm = get_between_range(fetched, 0, 3);
    instruction.A = get_bit_at(fetched, 21);
    instruction.S = get_bit_at(fetched, 20);
  } else {
    instruction.instr = DATA_PROCESS;
    instruction.op_code = get_between_range(fetched, 21, 24);
    instruction.Rn = get_between_range(fetched, 16, 19);
    instruction.Rd = get_between_range(fetched, 12, 15);
    instruction.I = get_bit_at(fetched, 25);
    instruction.S = get_bit_at(fetched, 20);
    if (instruction.I) {
      rotate = get_between_range(fetched, 8, 11) * 2;
      imm = get_between_range(fetched, 0, 7);
      int y = (int)((unsigned)imm >> rotate);
      int z = (int)((unsigned)imm << (32 - rotate));
      instruction.operand2 = y | z;
      instruction.temp_c = get_bit_at(instruction.operand2, 31);
    } else {
      instruction.operand2 = perform_shift(state, true, carry);
      instruction.temp_c = *carry;
    }
  }
  instruction.cond = get_between_range(fetched, 28, 31);
  instruction.raw_value = fetched;
  free(carry);	
  return instruction;
}

/*
  Executes an instruction returning true whenever the pipeline needs to be
  updated as a result of a non-linear PC register update.
*/
bool execute(struct State *state) {
  if (!check_condition(state) && state->decoded.instr != HALT) {
    return true;
  }

  switch (state->decoded.instr) {
  case HALT:
    state->running = false;
    break;
  case DATA_PROCESS:
    data_processing(state);
    break;
  case MULT:
    multiply(state);
    break;
  case TRANSFER:
    transfer(state);
    break;
  case BRANCH:
    branch(state);
    return false;
  }

  return true;
}

/*
  Prints current state to stdout formatted.
*/
void print_results(struct State *state) {
  printf("Registers:\n");
  for (int i = 0; i < 13; i++) {
    printf("$%-3d: %10d (0x%08x)\n", i, state->registers[i],
           state->registers[i]);
  }
  printf("PC  : %10d (0x%08x)\n", state->registers[PC], state->registers[PC]);
  printf("CPSR: %10d (0x%08x)\n", state->registers[CPSR],
         state->registers[CPSR]);
  printf("Non-zero memory:\n");

  for (int i = 0; i < CAPACITY; i += 4) {
    uint8_t *base = state->memory + i;
    unsigned int value =
        *(base) << 24 | *(base + 1) << 16 | *(base + 2) << 8 | *(base + 3);
    if (value) {
      printf("0x%08lx: 0x%08x\n", base - state->memory, value);
    }    
  }		
}
