#include "directive_processor.h"

struct Instruction *decode_directive(char *line){
  struct Instruction *instr = malloc(sizeof(struct Instruction));
  instr->instr_type = DIRECTIVE;
  instr->directive_arguments = calloc(MAX_DIRECTIVE_ARGS, sizeof(char *));
  instr->has_address = false;
  char *temp = calloc(strlen(line), sizeof(char));
  strcpy(temp, line);
  char *token;
  token = strtok(temp, " ");
  int i = 0;
  while(token != NULL){
    instr->directive_arguments[i] = calloc(strlen(token), sizeof(char));
    strcpy(instr->directive_arguments[i], token);
    token = strtok(NULL, " ");
    i++;
  }

  return instr;
}

void process_directive(struct Instruction *instr, struct Map *symbol_map, FILE *file){
  if(!strcmp(instr->directive_arguments[0], ".align")){
    uint32_t *temp = calloc(1, sizeof(uint32_t));
    *temp = atoi(instr->directive_arguments[1]);
    symbolv_t value = {.value_type = UINT32T, .uint_val = temp};
    add_to_map(symbol_map, CURRENT_ALIGNMENT_VAR, value);
  }else if(!strcmp(instr->directive_arguments[0], ".unreq")){
    char *temp = calloc(strlen(instr->directive_arguments[1]), sizeof(char));
    strcpy(temp, instr->directive_arguments[1]);
    strtok(temp, "\n");
    symbol_t symb = {.symbol_type = ASSEMBLER_VAR, .name = temp};
    if(!map_remove_element(symbol_map, symb)){
      print_map(symbol_map);
      printf("Unkown .unreq: '%s'", temp);
    }
  }else if(!strcmp(instr->directive_arguments[0], ".int")){
    int32_t *variable = calloc(1, sizeof(int32_t));
    *variable = atoi(instr->directive_arguments[1]);
    printf("PRINTING %d\n", *variable);

    uint32_t *current_address_ptr = get_value_from_map(symbol_map, CURRENT_ADDRESS)->uint_val;
    instr->has_address = true;
    instr->memory_address = *current_address_ptr;
    fseek(file, *current_address_ptr, SEEK_SET);
    fwrite(variable, 1, sizeof(uint32_t), file);
    *current_address_ptr += power(2, *get_value_from_map(symbol_map, CURRENT_ALIGNMENT_VAR)->uint_val);
  }else if(!strcmp(instr->directive_arguments[0], ".half")){
    int16_t *variable = calloc(1, sizeof(int16_t));
    *variable = atoi(instr->directive_arguments[1]);
    printf("PRINTING %d\n", *variable);
    uint32_t *current_address_ptr = get_value_from_map(symbol_map, CURRENT_ADDRESS)->uint_val;
    instr->has_address = true;
    instr->memory_address = *current_address_ptr;
    fseek(file, *current_address_ptr, SEEK_SET);
    fwrite(variable, 1, sizeof(uint32_t), file);
    *current_address_ptr += power(2, *get_value_from_map(symbol_map, CURRENT_ALIGNMENT_VAR)->uint_val);
  }else if(!strcmp(instr->directive_arguments[1], ".req")){
    char *temp = calloc(strlen(instr->directive_arguments[2]), sizeof(char));
    strcpy(temp, instr->directive_arguments[2]);
    strtok(temp, "\n");
    symbol_t symb = {.symbol_type = ASSEMBLER_VAR, .name = instr->directive_arguments[0]};
    symbolv_t val = {.value_type = STRING, .str_val = temp};
    add_to_map(symbol_map, symb, val);
  }
}
