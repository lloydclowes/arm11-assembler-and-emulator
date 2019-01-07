#ifndef ASSEMBLE_MAP_H__
#define ASSEMBLE_MAP_H__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim
*/

struct Map {
  int num_added;
  int size;
  struct KVPair *kv_array;
};

enum ValueType { UINT32T, STRING, POINTER};

enum SymbolType { LABEL, CONSTANT, VARIABLE, ASSEMBLER_VAR, INSTR_EVAL };
#define SYMBOL_TYPE_SIZE 4;

typedef struct Symbol {
  char *name;
  enum SymbolType symbol_type;
} symbol_t;

typedef struct SymbolValue {
  enum ValueType value_type;
  uint32_t *uint_val;
  char *str_val;
  void *ptr;
} symbolv_t;

typedef struct KVPair {
  symbol_t *symbol;
  symbolv_t *value;
} kvpair_t;

static const symbol_t FINAL_INSTR_ADDRESS = {
    .name = "final_instr_address",
    .symbol_type = ASSEMBLER_VAR
};

static const symbol_t INSTRUCTION_ARRAY = {
    .name = "instruction_array_ptr",
    .symbol_type = ASSEMBLER_VAR
};

static const symbol_t CURRENT_ADDRESS = {
    .name = "current_address",
    .symbol_type = ASSEMBLER_VAR
};

struct Map *make_new_map(int size);
symbolv_t *get_value_from_map(struct Map *map, symbol_t symbol);
void add_to_map(struct Map *map, symbol_t symbol, symbolv_t symbol_value);

bool map_contains(struct Map *map, symbol_t symbol);
bool map_remove_element(struct Map *map, symbol_t symbol);

int get_num_added(struct Map *map);
kvpair_t *get_all(struct Map *map);
void free_map(struct Map *map);
/* Debug */
void print_map(struct Map *map);
int count_lines(FILE *file);
int cmp_symbols(symbol_t a, symbol_t b);

#endif
