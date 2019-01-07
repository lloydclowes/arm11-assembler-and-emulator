#include "instructions.h"

static const char *Opcodes[7][11] = {
  {"add", "sub", "rsb", "and", "eor", "orr", "mov", "tst", "teq", "cmp", "mvn"},
  {"mul", "mla", "", "", "", "", "", "", "", "", ""},
  {"ldr", "str", "", "", "", "", "", "", "", "", ""},
  {"b", "bl", "bx", "", "", "", "", "", "", "", ""},
  {"ldm", "pop", "", "", "", "", "", "", "", "", ""},
  {"stm", "push", "", "", "", "", "", "", "", "", ""},
  {"lsl", "lsr", "", "", "", "", "", "", "", "", ""}
};

/*
  Turns a decoded data process instruction to binary
*/
uint32_t *data_process(struct Instruction instr, struct Map *symbol_map) {
  symbol_t label_symb = {.symbol_type = LABEL, .name = instr.operand2};
  uint32_t offset = 0;
  if(map_contains(symbol_map, label_symb)){
    symbolv_t *label_val = get_value_from_map(symbol_map, label_symb);
    struct Instruction *instr_arr = (struct Instruction *)get_value_from_map(symbol_map, INSTRUCTION_ARRAY)->ptr;
    offset = instr_arr[*label_val->uint_val].memory_address;
  }else{
    char *temp = malloc(sizeof(instr.operand2));
    strcpy(temp, instr.operand2);
    strtok(temp, "\n");
    symbol_t symbol = {.symbol_type = ASSEMBLER_VAR, .name = temp};
    if(map_contains(symbol_map, symbol)){
      instr.operand2 = get_value_from_map(symbol_map, symbol)->str_val;
    }
    offset = decode_expression(instr.operand2, symbol_map);
  }

  uint8_t cond = instr.cond;
  uint8_t opcode;
  bool S = 0;
  bool I = 1;
  printf("operand2: %s\n", instr.operand2);
  if (instr.operand2[0] == 'r') {
    I = 0;
  } else if (offset > 0xFF) {
    offset = get_operand2(offset);
  }
  printf("I: %i\n", I);

  uint32_t *instruction = malloc(sizeof(uint32_t));
  if (instruction == NULL) {
    allocate_error("Instruction allocation in data_process() failed");
  }
  if (!strcmp(instr.opcode, "and")) {
    opcode = 0;
  } else if (!strcmp(instr.opcode, "eor")) {
    opcode = 1;
  } else if (!strcmp(instr.opcode, "sub")) {
    opcode = 2;
  } else if (!strcmp(instr.opcode, "rsb")) {
    opcode = 3;
  } else if (!strcmp(instr.opcode, "add")) {
    opcode = 4;
  } else if (!strcmp(instr.opcode, "orr")) {
    opcode = 12;
  } else if (!strcmp(instr.opcode, "mov")) {
    opcode = 13;
  } else if (!strcmp(instr.opcode, "tst")) {
    opcode = 8;
    S = 1;
  } else if (!strcmp(instr.opcode, "teq")) {
    opcode = 9;
    S = 1;
  } else if (!strcmp(instr.opcode, "cmp")) {
    opcode = 10;
    S = 1;
  } else if (!strcmp(instr.opcode, "mvn")) {
    opcode = 15;
  }
  *instruction = (cond << 28) | (I << 25) | (opcode << 21) | (S << 20) |
                            (decode_expression(instr.Rn, symbol_map) << 16) | (decode_expression(instr.Rd, symbol_map) << 12) |
                            offset;
  printf("PROCESS INSTRUCTION COND: %i, OPCODE: %s", instr.cond, instr.opcode);
  return instruction;
}

/*
  Turns a decoded branch instruction to binary
*/
uint32_t *branch(struct Instruction *instr, struct Map *symbol_map) {
  uint8_t condition_code;
  uint32_t *res = calloc(1, sizeof(uint32_t));

  // Normal branch opcodes are length 3, so if length = 4 then it is branch with
  // link
  bool L = 0;
  if(strlen(instr->opcode) == 4 || !strcmp(instr->opcode, "bl")){
    L = 1;
  }

  if(!strcmp(instr->opcode, "bx")){
    condition_code = 14;
    *res = condition_code << 28 | 0x12FFF1 << 4 | decode_expression(instr->Rn, symbol_map);
    return res;
  }
  condition_code = instr->cond;
  condition_code <<= 4;
  condition_code |= 10 + L;

  struct Instruction *instr_arr = (struct Instruction *)get_value_from_map(symbol_map, INSTRUCTION_ARRAY)->ptr;

  /* Might need to add/subtract 8 to account for the pipeline */
  int offset;
  symbol_t label_symb = {.name = instr->expression, .symbol_type = LABEL};
  symbolv_t *label_val = get_value_from_map(symbol_map, label_symb);
  if(label_val != NULL && (instr_arr[*(label_val->uint_val)]).has_address){
    printf("Map Contains\n");
    offset = instr_arr[*label_val->uint_val].memory_address;
  }else{
    printf("Map NOT Contains\n");
    char *name = calloc(20, sizeof(char));
    sprintf(name, "%i", instr->memory_address);
    symbol_t branch_eval_symb =  {.symbol_type = INSTR_EVAL, .name = name};
    symbolv_t branch_eval_val = {.value_type = POINTER, .ptr = instr};
    add_to_map(symbol_map, branch_eval_symb, branch_eval_val);
    *res = 0;
    return res;
  }
  offset = offset - (int)(instr->memory_address);
  offset -= 8;
  offset = offset >> 2;
  uint32_t mask = 0xFFFFFF;
  offset &= mask;
  *res = ((condition_code << 24) | offset);
  return res;
}

uint32_t *shift(struct Instruction instr, struct Map *symbol_map){
  uint32_t *res = calloc(1, sizeof(uint32_t));
  uint8_t cond = 14;
  uint8_t opcode = 13;

  uint32_t Rn = decode_expression(instr.Rn, symbol_map);
  uint32_t shift_amount = decode_expression(instr.expression, symbol_map);
  uint8_t S = 0;

  /* Implicitly leave I, bits 4-6, 12-15 as 0 */
  *res = ((cond << 28) | (opcode << 21) | (S << 20) | Rn << 12 | (shift_amount << 7));

  if (!strcmp(instr.opcode, "lsr")) {
    *res |= (1 << 5);
  }
  *res |= Rn;
  return res;
}

/*
  Turns a decoded multiply instruction into binary
*/
uint32_t *multiply(struct Instruction instr, struct Map *symbol_map) {

  uint16_t mul_field = 9;
  uint16_t cond = instr.cond;
  uint16_t a;
  uint16_t rd = decode_expression(instr.Rd, symbol_map);
  uint16_t rm = decode_expression(instr.Rm, symbol_map);
  uint16_t rs = decode_expression(instr.Rs, symbol_map);
  uint16_t rn;

  uint32_t *res = malloc(sizeof(uint32_t));
  if (res == NULL) {
    allocate_error("Result allocation in multiply() failed");
  }

  if(!strcmp(instr.opcode, "mul")) {
    a = 0;
    rn = 0;
  } else if (!strcmp(instr.opcode, "mla")) {
    a = 1;
    rn = decode_expression(instr.Rn, symbol_map);
  } else {
    printf("error: opcode doesn't match multiply\n");
    printf("OPCODE: %s\n", instr.opcode);
    exit(EXIT_FAILURE);
  }

  *res = (cond << 28) | (a << 21) | (rd << 16) | (rn << 12) |
    (rs << 8) | (mul_field << 4) | rm;
  return res;
}

/*
  Turns a decoded transfer instruction to binary
*/
uint32_t *transfer(struct Instruction *instr, struct Map *symbol_map) {

  if(!map_contains(symbol_map, FINAL_INSTR_ADDRESS)){
    char *name = calloc(20, sizeof(char));
    sprintf(name, "%i", instr->memory_address);
    symbol_t symbol = {.name = name, .symbol_type = INSTR_EVAL};
    symbolv_t symb_val = {.value_type = POINTER, .ptr = instr};
    add_to_map(symbol_map, symbol, symb_val);
    uint32_t *res = calloc(1, sizeof(uint32_t));
    *res = 0;
    return res;
  }

  /* decode address */

  char *address = instr->address;
  char *new_address = malloc(sizeof(char) * strlen(address));
  if (new_address == NULL) {
    allocate_error("New address allocation in transfer() failed");
  }
  char *temp1;

  const char ldr[] = "ldr";
  const char str[] = "str";
  bool p_flag, i_flag, u_flag, l_flag;
  int value;
  uint16_t rd = decode_expression(instr->Rd, symbol_map);
  uint16_t rn;
  int32_t offset;
  uint16_t cond = 0xe;
  char *tokenised_address[3];

  char *base;
  char *shift_type;
  char *shift_amount;

  uint32_t *res = malloc(sizeof(uint32_t));
  if (res == NULL) {
    allocate_error("Result allocation in transfer() failed");
  }

  if(!strcmp(instr->opcode, ldr)) {

    l_flag = 1;
    if (address[0] == '=') { /* check if there is = */
      p_flag = 1;
      value = decode_expression(address, symbol_map);
      if (value < 0xFF) {
        instr->opcode = "mov";
        instr->operand2 = address;
        instr->Rn = "#0";
        return data_process(*instr, symbol_map);
      } else {
        i_flag = 0;
        u_flag = 1;
        uint32_t pc = instr->memory_address + 8;
        printf("PC is %d\n", pc);
        rn = 15; /* #define PC 15 in emulate */
        offset = calculate_offset(pc, value, symbol_map);
        *res =  (cond << 28) | (1 << 26) | (i_flag << 25) | (p_flag << 24) | (u_flag << 23)
          | (l_flag << 20) | (rn << 16) | (rd << 12) | offset;
        return res;
      }
    }
  } else if (!strcmp(instr->opcode, str)) {
    l_flag = 0;
  } else {
    printf("error: opcode doesn't match transfer\n");
    exit(EXIT_FAILURE);
  }

  strcpy(new_address, address); /* can't use strtok on string literal */
  p_flag = (new_address[strlen(new_address) - 1] == ']') ? 1 : 0;

  int i = 0;
  temp1 = strtok(new_address, ",");

  while(temp1) {
    tokenised_address[i++] = temp1;
    temp1 = strtok(NULL, ",");
  }

  switch(i) {
    case 1:
      i_flag = 0;
      offset = 0;
      u_flag = 1;
      /* get rid of ] in the last character of the string */
      tokenised_address[0][strlen(tokenised_address[0]) - 1] = '\0';
      instr->Rn = tokenised_address[0];
      rn = decode_expression(instr->Rn, symbol_map);
      break;
    case 2:
      if(tokenised_address[1][0] == ' ') { /* tokenised_address[i-1][0] */
        tokenised_address[1]++;
      }
      if (p_flag) {
        tokenised_address[1][strlen(tokenised_address[1]) - 1] = '\0';
      } else {
        tokenised_address[0][strlen(tokenised_address[0]) - 1] = '\0';
      }
      i_flag = !is_immediate(tokenised_address[1]);
      u_flag = 1;
      offset = decode_expression(tokenised_address[1], symbol_map);
      if (offset < 0) {
        u_flag = 0;
        offset *= -1;
      }
      instr->Rn = tokenised_address[0];
      rn = decode_expression(instr->Rn, symbol_map);
      break;
    case 3:
      base = tokenised_address[1];
      shift_type = strtok(tokenised_address[2], " ");
      shift_amount = strtok(NULL, " ");
      if (p_flag) {
        if (shift_amount[strlen(shift_amount) - 1] == ']') {
          shift_amount[strlen(shift_amount) - 1] = '\0';
        }
      } else {
        if (base[strlen(base) - 1] == ']') {
          base[strlen(base) - 1] = '\0';
        }
      }

      offset = shift_operand2(base, shift_type, shift_amount, symbol_map);
      u_flag = 1;
      if (offset < 0) {
        u_flag = 0;
        offset *= -1;
      }
      i_flag = !is_immediate(base);
      instr->Rn = tokenised_address[0];
      rn = decode_expression(instr->Rn, symbol_map);
  }



  *res = (cond << 28) | (1 << 26) | (i_flag << 25) | (p_flag << 24) | (u_flag << 23)
    | (l_flag << 20) | (rn << 16) | (rd << 12) | offset;
  free(new_address);
  return res;
}

// returns a negative number if the argument is not an opcode
bool check_if_opcode(char *opcode) {
  int len = sizeof(Opcodes[0]) / sizeof(Opcodes[0][0]);
  for(int i = 0; i < 7; i++){
    for (int j = 0; j < len; j++){
      if(!strcmp(opcode, Opcodes[i][j])){
        return true;
      }
    }
  }
  return false;
}

// returns a negative number if the argument is not an opcode
enum InstrType get_instr_type(char *opcode) {
  int len = sizeof(Opcodes[0]) / sizeof(Opcodes[0][0]);
  for(int i = 0; i < 7; i++){
    for (int j = 0; j < len; j++){
      if(!strcmp(opcode, Opcodes[i][j])){
        return i;
      }
    }
  }
  return DATA_PROCESSING;
}

int32_t calculate_offset(uint32_t PC, uint32_t value, struct Map *symbol_map) {

  int32_t offset;
  char *key = malloc(sizeof(char) * 10); /* need to change it. magic number */
  if (key == NULL) {
    allocate_error("Key allocation in calculate_offset() failed");
  }
  symbol_t symbol;
  int i = 0;
  do{
    sprintf(key, "%i", i);
    symbol = (symbol_t){.name = key, .symbol_type = CONSTANT};
    i++;
  }while(map_contains(symbol_map, symbol));
  uint32_t *temp_val = calloc(1, sizeof(uint32_t));
  *temp_val = value;
  symbolv_t symbol_value = {.value_type = UINT32T, .uint_val = temp_val};
  add_to_map(symbol_map, symbol, symbol_value);
  uint32_t last_instr = *(get_value_from_map(symbol_map, FINAL_INSTR_ADDRESS)->uint_val);
  printf("key is %s\n", key);
  printf("last instr is %d\n", last_instr);
  offset = (int32_t)last_instr - (int32_t)PC + ((i - 1) * 4);
  printf("OFFSET FOR PC : %d  is : %d\n", PC, offset);
  free(key);
  return offset;
}
