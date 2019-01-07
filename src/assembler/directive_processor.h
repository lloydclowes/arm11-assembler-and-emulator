#ifndef DIRECTIVE_PROCESSOR_H__
#define DIRECTIVE_PROCESSOR_H__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "utilities.h"
#include "instructions.h"
/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim
*/
#define MAX_DIRECTIVE_ARGS 3
static const symbol_t CURRENT_ALIGNMENT_VAR = {
    .name =  "current_alignment_var",
    .symbol_type = ASSEMBLER_VAR
};

struct Instruction *decode_directive(char *line);
void process_directive(struct Instruction *instr, struct Map *symbol_map, FILE *file);

#endif
