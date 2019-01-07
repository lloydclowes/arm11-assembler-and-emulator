#include "assemble.h"

int main(int argc, char **argv) {

  FILE *input;
  if ((input = fopen(argv[1], "r")) == NULL) {
    printf("Could not open %s!", argv[1]);
  }

  int instruction_count = count_lines(input);
  rewind(input);

  struct Instruction *instr_arr =
      calloc(instruction_count, sizeof(struct Instruction));
  uint32_t memCount = 0;
  struct Map *symbol_map = make_new_map(instruction_count + RESERVED_FOR_ASSEMBLER);

  symbolv_t instr_arr_val = {.value_type = POINTER, .ptr = instr_arr};
  add_to_map(symbol_map, INSTRUCTION_ARRAY, instr_arr_val);

  int instr_arr_size = 0;

  /* First Pass */
  char line_buffer[MAX_LINE_SIZE];
  while (fgets(line_buffer, sizeof(line_buffer), input)) {

    /* Remove comments */
    char COMMENT_START = '@';
    char *comment_pos = strchr(line_buffer, COMMENT_START);
    if (comment_pos != NULL) {
      /* A comment exists beginning at *comment_pos */
      if (comment_pos == line_buffer) {
        /* The whole line is a comment so ignore line */
        printf("Removing the whole line\n");
        continue;
      }
      /* Comment starts after some code so replace COMMENT_START with '\0' */
      memset(comment_pos, '\0', 1);
    }

    /* Remove empty lines */
    if (strlen(line_buffer) == 1 && !strcmp(line_buffer, "\n")) {
      printf("Removing line");
      continue;
    }
    printf("LINE BUFFER: %s\n", line_buffer);
    if(strchr(line_buffer, '.')){
      /*IS DIRECTIVE*/
      instr_arr[instr_arr_size] = *decode_directive(line_buffer);
      instr_arr_size++;
      continue;
    }
    char **tokenised_line = tokenise_line(line_buffer);

    if (tokenised_line[0][strlen(tokenised_line[0]) - 1] == ':') {
      /* Is Label */
      printf("LABEL line[0] %s\n", tokenised_line[0]);
      char *label = malloc(sizeof(tokenised_line[0]));
      strcpy(label, tokenised_line[0]);
      free(tokenised_line[0]);
      label[strlen(label) - 1] = 0;
      symbol_t label_symbol = {.name = label, .symbol_type = LABEL};
      if (!map_contains(symbol_map, label_symbol)) {
        printf("ADDED %s\n", label);
        uint32_t *val = calloc(1, sizeof(uint32_t));
        *val = instr_arr_size;
        symbolv_t value = {.uint_val = val, .value_type = UINT32T};
        add_to_map(symbol_map, label_symbol, value); /* 4 bytes per line */
      }

      printf("PRINTING MAP : \n");
      print_map(symbol_map);
    } else {
      /* Is Instruction */
      struct Instruction *instruction = decode(tokenised_line);
      instr_arr[instr_arr_size] = *instruction;
      instr_arr_size++;
    }
    free(tokenised_line);
  }

  fclose(input);

  FILE *output;
  if ((output = fopen(argv[2], "wb")) == NULL) {
    printf("Could not open %s!\n", argv[2]);
  }

  /* Second pass needs to be altered to allow for alignments etc */
  /* Second Pass */

  /*
  int alignment = 2;
  i += 8 * (power(2, alignment))
  */
  uint32_t *current_address_ptr = malloc(sizeof(uint32_t));
  *current_address_ptr = 0;
  symbolv_t current_address_val = {.value_type = UINT32T, .uint_val = current_address_ptr};
  add_to_map(symbol_map, CURRENT_ADDRESS, current_address_val);
  print_map(symbol_map);


  uint32_t *alignment = malloc(sizeof(uint32_t));
  *alignment = 2;
  symbolv_t current_alignment_var = {.value_type = UINT32T, .uint_val = alignment};
  add_to_map(symbol_map, CURRENT_ALIGNMENT_VAR, current_alignment_var);
  print_map(symbol_map);

  uint32_t *last_valid_instr = calloc(1, sizeof(uint32_t));
  *last_valid_instr = 0;

  for (int i = 0; i < instr_arr_size; i++) {
    current_address_ptr = get_value_from_map(symbol_map, CURRENT_ADDRESS)->uint_val;
    alignment = get_value_from_map(symbol_map, CURRENT_ALIGNMENT_VAR)->uint_val;
    if(instr_arr[i].instr_type == DIRECTIVE){
      process_directive((struct Instruction *)(instr_arr + i), symbol_map, output);
      continue;
    }
    uint32_t *bin = to_binary((instr_arr + i), symbol_map, *current_address_ptr);
    *last_valid_instr = *current_address_ptr;
    printf("Writing %x, to %x\n", *bin, *current_address_ptr);
    fseek(output, *current_address_ptr, SEEK_SET);
    fwrite(bin, power(2, *alignment), 1, output);
    free(bin);
    *current_address_ptr += power(2, *alignment);
  }
  *last_valid_instr += 4;
  symbolv_t last_instr_val = {.value_type = UINT32T, .uint_val = last_valid_instr};
  add_to_map(symbol_map, FINAL_INSTR_ADDRESS, last_instr_val);

  /* Evaluate all unresolved instructions */
  kvpair_t *all_symbols = get_all(symbol_map);
  for(int i = 0; i < symbol_map->size; i++){
    if(all_symbols[i].symbol == NULL || all_symbols[i].value == NULL){
      continue;
    }
    if(all_symbols[i].symbol->symbol_type != INSTR_EVAL){
      continue;
    }
    int memory_address = atoi(all_symbols[i].symbol->name);
    struct Instruction *instr = (struct Instruction *)all_symbols[i].value->ptr;
    printf("EVALUATING BRANCH INSTRUCTION AGAIN\n");
    uint32_t *bin = to_binary(instr, symbol_map, instr->memory_address);
    printf("VALUE TO WRITE: %x \n", *bin);
    fseek(output, (memory_address), SEEK_SET);
    fwrite(bin, 1, sizeof(uint32_t), output);
    printf("RE-Writing %x, to %x\n", *bin, memory_address);
  }
  /*
    Write constant_map to end of file
  */

  kvpair_t *kv_pairs = get_all(symbol_map);
  fseek(output, *last_valid_instr, SEEK_SET);
  for (int i = 0; i < symbol_map->size; i++) {
    kvpair_t kv = kv_pairs[i];
    if (kv.symbol == NULL || kv.value == NULL) {
      continue;
    }
    if (kv.symbol->symbol_type == CONSTANT) {
      printf("***Writing*** %x\n", *kv.value->uint_val);
      fwrite(kv.value->uint_val, 1, sizeof(uint32_t), output);
    }
  }

  printf("Done adding to end\n\n");

  fclose(output);
  // Free variables
  for (int i = 0; i < memCount; i++) {
    free_instruction(instr_arr[i]);
  }
  free(instr_arr);
  free_map(symbol_map);
  return EXIT_SUCCESS;
}

bool is_directive(char *string) { return string[0] == '.'; }

/*
    Decodes the given instruction (as a string) and returns an Instruction
   structure
 */
struct Instruction *decode(char **line) {
  struct Instruction *instr = malloc(sizeof(struct Instruction));
  if (instr == NULL) {
    allocate_error("Instruction allocation in decode() failed");
  }

  instr->opcode = malloc((sizeof(line[0]) + 1) * sizeof(char));
  if (instr->opcode == NULL) {
    allocate_error("Instruction opcode allocation in decode() failed");
  }

  *instr = split_opcode(*instr, line[0]);

  printf("Opcode: %s\n", instr->opcode);
  printf("Instr type: %i\n", instr->instr_type);
  printf("Cond: %i\n", instr->cond);
  printf("Stack mode: %s\n\n", instr->stack_mode);
  instr->has_address = false;
  int i = 1;
  int line_len = sizeof(line[0]) / sizeof(line[0][0]);
  switch (instr->instr_type) {
  case DATA_PROCESSING:
    if (!strcmp(line[0], "mov")) {
      instr->Rd = line[1];
      instr->Rn = "#0";
      instr->operand2 = line[2];
    } else if (!strcmp(line[0], "tst") || !strcmp(line[0], "teq") ||
               !strcmp(line[0], "cmp")) {
      instr->Rd = "#0";
      instr->Rn = line[1];
      instr->operand2 = line[2];
    } else {
      instr->Rd = line[1];
      instr->Rn = line[2];
      instr->operand2 = line[3];
    }

    break;
  case MULTIPLY:
    printf("MULTIPLY\n");
    instr->Rd = line[1];
    instr->Rm = line[2];
    instr->Rs = line[3];
    if (!strcmp(line[0], "mla")) {
      instr->Rn = line[4];
    }
    break;
  case SINGLE_DATA_TRANSFER:
    instr->Rd = line[1];
    instr->address = line[2];
    break;
  case BRANCH:
    if (!strcmp(line[0], "bx")) {
      instr->Rn = line[1];
    } else {
      instr->expression = line[1];
    }
    break;
  case STACK_LOAD:
  case STACK_STORE:
    if (strcmp(instr->opcode, "pop") && strcmp(instr->opcode, "push")) {
      instr->Rn = line[1];
      i++;
    }
    instr->Rlist = calloc(16, sizeof(char *));
    for (; i < line_len; i++) {
      if (strlen(line[i]) == 0) {
        i = line_len;
      } else {
        instr->Rlist[i - 1] = malloc(sizeof(line[i]));
        strcpy(instr->Rlist[i - 1], line[i]);
      }
    }
    break;
  case SHIFT:
    instr->Rn = line[1];
    instr->expression = line[2];
    break;
  case DIRECTIVE:
    break;
  }
  // for (int i = 0; i < MAX_ARGS_NUM; i++) {
  //   free(line[i]);
  // }
  return instr;
}

/*
  Take in the decoded instruction and convert it to binary by passing
    it onto a function specific to the instruction type
*/
uint32_t *to_binary(struct Instruction *instr, struct Map *symbol_map,
                    uint32_t memory_address) {
  uint32_t *binary_instr;
  instr->memory_address = memory_address;
  instr->has_address = true;
  switch (instr->instr_type) {
  case DATA_PROCESSING:
    binary_instr = data_process(*instr, symbol_map);
    break;
  case MULTIPLY:
    binary_instr = multiply(*instr, symbol_map);
    break;
  case SINGLE_DATA_TRANSFER:
    binary_instr = transfer(instr, symbol_map);
    break;
  case BRANCH:
    binary_instr = branch(instr, symbol_map);
    break;
  case STACK_LOAD:
    binary_instr = stack_load(*instr, symbol_map);
    break;
  case STACK_STORE:
    binary_instr = stack_store(*instr, symbol_map);
    break;
  case SHIFT:
    binary_instr = shift(*instr, symbol_map);
    break;
  case DIRECTIVE:
    perror("INVALID DIRECTIVE INSTRUCTION");
    break;
  }
  return binary_instr;
}

// takes in the opcode and sets the instr opcode, cond and stack_mode where
// necessary
struct Instruction split_opcode(struct Instruction instr, char *opcode) {
  printf("SPLITTING OPCODE FOR %s\n", opcode);
  size_t len = strlen(opcode);
  instr.cond = 14;
  instr.stack_mode = NULL;

  if (len <= 2) {
    // doesn't have condition at the end
    instr.opcode = opcode;
    instr.instr_type = get_instr_type(instr.opcode);
    return instr;
  }

  instr.opcode = malloc(4);

  // check if the opcode is 'ldm' or 'stm'
  if (is_stack_operation(opcode)) {

    if (strlen(opcode) >= 4) {
      char *temp = malloc(4);
      strncpy(temp, opcode, 4);
      if (!strcmp(temp, "push")) {
        strncpy(instr.opcode, opcode, 4);
      } else {
        strncpy(instr.opcode, opcode, 3);
      }
      free(temp);
    } else {
      strncpy(instr.opcode, opcode, 3);
    }

    if (strlen(opcode) >= 5) {
      instr.stack_mode = malloc(2);
      strncpy(instr.stack_mode, opcode + len - 2, 2);
    }
    if (len > 5) {
      // has a condition code
      char *cond_code = malloc(2);
      strncpy(cond_code, opcode + 3, 2);
      instr.cond = get_cond_code(cond_code);
      free(cond_code);
    }

    instr.instr_type = get_instr_type(instr.opcode);
    return instr;
  }

  uint8_t cond = get_cond_code(opcode + len - 2);

  if (cond < 0) {
    // doesn't have a condition at the end
    instr.opcode = opcode;
    instr.instr_type = get_instr_type(instr.opcode);
    return instr;
  }

  // by this point we are still unsure if it has a condition at the end
  // we need check if the opcode the potential condition at the end removed
  //  is an opcode
  char *opcode_cpy = malloc((len - 2) * sizeof(char));
  strncpy(opcode_cpy, opcode, len - 2);

  if (check_if_opcode(opcode_cpy)) {
    // it has a condition at the end
    instr.opcode = opcode_cpy;
    instr.instr_type = get_instr_type(instr.opcode);
    instr.cond = cond;
  } else {
    // doesn't have a condition at the end
    instr.opcode = opcode;
    instr.instr_type = get_instr_type(instr.opcode);
    free(opcode_cpy);
  }

  return instr;
}

void free_instruction(struct Instruction instr) {
  free(instr.stack_mode);
  free(instr.opcode);
  free(instr.address);
  free(instr.expression);
  free(instr.operand2);
  free(instr.Rm);
  free(instr.Rs);
}
