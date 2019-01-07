#include "stack_instructions.h"

uint32_t *stack_store(struct Instruction instr, struct Map *symbol_map){
  uint8_t cond = 0xE;
  uint32_t *res_ptr = calloc(1, sizeof(uint32_t));
  uint32_t res = 0;
  if(instr.cond != -1){
      cond = instr.cond;
  }

  if(!strcmp(instr.opcode, "push")){
    res = cond;
    res <<= 28;
    res |= (0x92D << 16);
    res |= stack_get_Rlist(instr.Rlist, symbol_map);
    *res_ptr = res;
    return res_ptr;
  }

  bool L = false;
  bool P = instr.opcode[3] == 'f' || instr.opcode[4] == 'b';
  bool U = instr.opcode[4] == 'a' || instr.opcode[3] == 'i';

  bool S = (instr.Rn[strlen(instr.Rn) - 1] == '!');

  int Rlist_len = sizeof(instr.Rlist[0]) / sizeof(instr.Rlist[0][0]);
  char *lastVal = instr.Rlist[Rlist_len - 1];
  if(lastVal == NULL){
    perror("Could not tokenise input correctly!");
    return 0;
  }
  bool W = (lastVal[strlen(lastVal) - 1] == '^');

  uint8_t Rn = decode_expression(instr.Rn, symbol_map);

  res = cond;
  res <<= 28;
  res |= (3 << 25);
  res |= (P << 24);
  res |= (U << 23);
  res |= (S << 22);
  res |= (W << 21);
  res |= (L << 20);
  res |= (Rn << 16);
  res |= stack_get_Rlist(instr.Rlist, symbol_map);

  *res_ptr = res;
  return res_ptr;
}

uint32_t *stack_load(struct Instruction instr, struct Map *symbol_map){
  uint8_t cond = 0xE;
  uint32_t *res_ptr = calloc(1, sizeof(uint32_t));
  uint32_t res = 0;
  if(instr.cond != -1){
      cond = instr.cond;
  }
  if(!strcmp(instr.opcode, "pop")){
    res = cond;
    res <<= 28;
    res |= (0x8BD << 16);
    res |= stack_get_Rlist(instr.Rlist, symbol_map);
    printf("RETURNING with %x\n", res);
    *res_ptr = res;
    return res_ptr;
  }

  bool L = true;
  bool P = (instr.opcode[3] == 'e' || instr.opcode[4] == 'b');
  bool U = (instr.opcode[4] == 'd' || instr.opcode[3] == 'i');

  bool S = (instr.Rn[strlen(instr.Rn) - 1] == '!');


  int Rlist_len = sizeof(instr.Rlist[0]) / sizeof(instr.Rlist[0][0]);
  char *lastVal = instr.Rlist[Rlist_len - 1];
  if(lastVal == NULL){
    perror("Could not tokenise input correctly!");
    return 0;
  }

  bool W = (lastVal[strlen(lastVal) - 1] == '^');

  uint8_t Rn = decode_expression(instr.Rn, symbol_map);
  res = cond;
  res <<= 28;
  res |= (3 << 25);
  res |= (P << 24);
  res |= (U << 23);
  res |= (S << 22);
  res |= (W << 21);
  res |= (L << 20);
  res |= (Rn << 16);
  res |= stack_get_Rlist(instr.Rlist, symbol_map);
  *res_ptr = res;
  return res_ptr;
}

uint32_t stack_get_Rlist(char **Rlist, struct Map *symbol_map){

    int Rlist_len = sizeof(Rlist[0]) / sizeof(Rlist[0][0]);
    uint32_t res = 0;
    for(int i = 0; i < Rlist_len; i++){
      if(Rlist[i] == NULL){
        return res;
      }
      char *token = malloc(sizeof(Rlist[i]));
      if(token == NULL){
        perror("Allocation Error!\n");
      }
      strcpy(token, Rlist[i]);

      char *char_ptr;
      if((char_ptr = strchr(token, '{')) != NULL){
        int index = (int)(char_ptr - token);
        memmove(&token[index], &token[index + 1], strlen(token) - index);
      }
      if((char_ptr = strchr(token, '}')) != NULL){
        int index = (int)(char_ptr - token);
        memmove(&token[index], &token[index + 1], strlen(token) - index);
      }
      if(strchr(token, '-') != NULL){
        char *range_1 = strtok(token, "-");
        char *range_2 = (char *)(token + 3);
        uint32_t register_1 = decode_expression(range_1, symbol_map);
        uint32_t register_2 = decode_expression(range_2, symbol_map);
        assert(register_1 <= register_2);
        for(int i = register_1; i <= register_2; i++){
          res |= (1 << i);
        }
      }else{
        uint32_t reg = decode_expression(token, symbol_map);
        res |= (1 << reg);
      }
    }
    return res;
}

bool is_stack_operation(char *opcode) {
  assert(strlen(opcode) >= 3);

  char *first_ones = malloc(5 * sizeof(char));

  strncpy(first_ones, opcode, 3);
  first_ones[3] = '\0';

  enum InstrType type = get_instr_type(first_ones);

  if (type == 4 || type == 5) {
    free(first_ones);
    return true;
  } else if (strlen(opcode) >= 4) {
    first_ones[3] = opcode[3];
    first_ones[4] = '\0';
    if (!strcmp(first_ones, "push")) {
      free(first_ones);
      return true;
    } else {
      free(first_ones);
      return false;
    }
  }
  free(first_ones);
  return false;
}
