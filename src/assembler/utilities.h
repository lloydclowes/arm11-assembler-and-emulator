#ifndef ASSEMBLE_UTILITIES_H__
#define ASSEMBLE_UTILITIES_H__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

#define MAX_ARGS_NUM 18
#define MAX_LINE_SIZE 511

#define CAPACITY 1 << 16

char **tokenise_line(char *line);
uint32_t get_operand2(int32_t operand2);
uint8_t get_cond_code(char *cond);
char *remove_tabs(char *string);
bool is_comment_start(char *string);
bool is_comment_end(char *string);
int count_lines(FILE *file);
int32_t decode_expression(char *expr, struct Map *symbol_map);
uint32_t shift_operand2(char *base, char* shift_type, char* shift, struct Map *symbol_map);
bool is_immediate(char *operand2);
void allocate_error(char message[]);
int power(int x, int y);
#endif
