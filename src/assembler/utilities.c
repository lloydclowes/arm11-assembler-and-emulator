#include "utilities.h"

/*
  Take in a string and first tokenise it by the '[' character (for further
    processing in the transfer function) and then by the space and comma
    characters (to split up the instruction into the arguments)
*/
char **tokenise_line(char *line) {
  char *temp1;
  char *temp2 = NULL;
  char new_line[MAX_LINE_SIZE];
  char **tokenised = malloc(sizeof(char *) * MAX_ARGS_NUM);
  if (tokenised == NULL) {
    allocate_error("Tokenised string allocation in tokenise_line() failed");
  }
  int i = 0;

  if(line[0] == ' ') {
    strcpy(new_line, line + 1);
  } else {
    strcpy(new_line, line);
  }

  if (line[strlen(line) - 1] == '\n') {
    line[strlen(line) - 1] = '\0';
  }

  strcpy(new_line, line); /* can't use strtok on string literal */
  if(strchr(line, '[') != NULL) {
    temp1 = strtok(new_line, "[");
    temp2 = strtok(NULL, "[");
  } else {
    temp1 = new_line;
  }

  temp1 = strtok(temp1, ", ");

  while(temp1) {
    tokenised[i] = malloc(sizeof(char *) * (strlen(temp1) + 1));
    if (tokenised[i] == NULL) {
      allocate_error("tokenise_line");
    }
    strcpy(tokenised[i++], temp1);
    temp1 = strtok(NULL, ", ");
  }
  if ((temp2 != NULL) && i < MAX_ARGS_NUM) {
    tokenised[i] = malloc(sizeof(char *) * (strlen(temp2) + 1));
    strcpy(tokenised[i++], temp2);
  }
  while (i < MAX_ARGS_NUM) {
    tokenised[i++] = "";
  }
  return tokenised;
}

/*
  Takes in an integer and returns the shifted integer as well as the
    amount it was shifted by in order to decrease the size of it
  Used by data_process() when operand2 is bigger than what can be stored
    in the instruction
*/
uint32_t get_operand2(int32_t operand2) {

  uint16_t shift_amount = 0;
  uint32_t temp = operand2;
  while (!(temp % 4)) {
    temp /= 4;
    shift_amount += 2;
  }

  int y = (int)((unsigned) operand2 >> shift_amount);
  int z = operand2 << (32 - shift_amount);
  uint32_t res = y | z;

  shift_amount = (32 - shift_amount) / 2;

  return (shift_amount << 8)| res;
}


// returns the condition code associated with the string inputted
// returns a negative number if it is not a condition code
uint8_t get_cond_code(char *cond) {
  if (!strcmp(cond, "eq")) {
    return 0;
  } else if (!strcmp(cond, "ne")) {
    return 1;
  } else if (!strcmp(cond, "cs") || !strcmp(cond, "ms")) {
    return 2;
  } else if (!strcmp(cond, "cc") || !strcmp(cond, "lo")) {
    return 3;
  } else if (!strcmp(cond, "mi")) {
    return 4;
  } else if (!strcmp(cond, "pl")) {
    return 5;
  } else if (!strcmp(cond, "vs")) {
    return 6;
  } else if (!strcmp(cond, "vc")) {
    return 7;
  } else if (!strcmp(cond, "hi")) {
    return 8;
  } else if (!strcmp(cond, "ls")) {
    return 9;
  } else if (!strcmp(cond, "ge")) {
    return 10;
  } else if (!strcmp(cond, "lt")) {
    return 11;
  } else if (!strcmp(cond, "gt")) {
    return 12;
  } else if (!strcmp(cond, "le")) {
    return 13;
  } else if (!strcmp(cond, "l")) {
    return 14;
  } else {
    return -1;
  }
}

char *remove_tabs(char *string) {
  int i;
  for (i = 0; string[i] == '\t'; i++);
  return string + i;
}

bool is_comment_start(char *string) {
  return string[0] == '/' && string[1] == '*';
}

bool is_comment_end(char *string) {
  return string[strlen(string) - 2] == '*' && string[strlen(string) - 1] == '/';
}

int count_lines(FILE *file){
  int count = 0;
  char ch;
  // char line_buffer[MAX_LINE_SIZE];
  // while(fgets(line_buffer, sizeof(line_buffer), file)){
  //   count++;
  // }

  while(!feof(file)) {
    ch = fgetc(file);
    if (ch == '\n') {
      count++;
    }
  }
  return count;
}



uint32_t shift_operand2(char *base, char* shift_type, char* shift, struct Map *symbol_map) {

  uint32_t result = 0;
  int base_index = decode_expression(base, symbol_map);
  int shift_code = 0;
  int shift_amount;

  if (!strcmp(shift_type, "lsl")) {
    shift_code = 0;
  } else if (!strcmp(shift_type, "lsr")) {
    shift_code = 1;
  } else if (!strcmp(shift_type, "asr")) {
    shift_code = 2;
  } else if (!strcmp(shift_type, "ror")) {
    shift_code = 3;
  }

  if (is_immediate(shift)) {
    shift_amount = 7;
  } else {
    shift_amount = 8;
  }

  result =  (decode_expression(shift, symbol_map) << shift_amount) | (shift_code << 5)
   | ((shift_amount == 8) << 4) | (base_index);

  return result;
}


bool is_immediate(char *operand2) {
  switch(operand2[0]) {
    case 'r':
      /* return register index */
      return false;
    /* need to implement convert immediate to binary */
    case '#':
    case '=':
      return true;
  }
  return false;
}


/*
  Takes an error message and displays it before exiting the program
  Called whenever an allocation returns NULL
*/
void allocate_error(char message[]) {
  perror(message);
  exit(EXIT_FAILURE);
}

int power(int x, int y){
  if(y == 0){
    return 1;
  }
  return x * power(x, y - 1);
}


int32_t decode_expression(char *expr, struct Map *symbol_map) {
  char *temp = malloc(sizeof(expr));
  strcpy(temp, expr);
  strtok(temp, "\n");
  symbol_t symbol = {.symbol_type = ASSEMBLER_VAR, .name = temp};
  if(map_contains(symbol_map, symbol)){
    printf("replacing val %s ", temp);
    expr = get_value_from_map(symbol_map, symbol)->str_val;
    printf("With %s \n", expr);
  }
  if(!strcmp(expr, "sp")){
   return 13;
  }else if(!strcmp(expr, "lr")){
    return 14;
  }else if(!strcmp(expr, "pc")){
    return 15;
  }
  int32_t res = 0;
  int32_t i = 1;

  switch(expr[0]) {
    case 'r':
      /* return register index */
      res = atoi(expr+1);
      break;
    /* need to implement convert immediate to binary */
    case '#':
    case '=':
      if (expr[1] == '-') {
        i = 2;
      }
      if (expr[i] == '0' && expr[i + 1] == 'x') {
        /* Is of the form: #0x0F, =0x42 */
        res = (int32_t)strtol(expr + 1, NULL, 0);
      } else {
        /* Is of the form: #56 */
        res = atoi(expr+1);
      }
      break;
  }
  return res;
}
