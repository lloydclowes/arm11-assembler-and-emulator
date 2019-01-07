#include "map.h"
/*
  Summer Term 2018
  Authors: Group 37 - Lloyd Clowes, Hubert Kaluzny, Buneme Kyakilika, Jeewoo Kim
*/

struct Map *make_new_map(int size) {
  struct KVPair *kv_array = malloc(size * sizeof(kvpair_t));
  struct KVPair empty_kv = {.symbol = NULL, .value = NULL};
  /* Initialize all elements to null */
  for (int i = 0; i < size; i++) {
    kv_array[i] = empty_kv;
  }

  struct Map *map = malloc(sizeof(struct Map));
  map->kv_array = kv_array;
  map->size = size;
  map->num_added = 0;
  return map;
}

kvpair_t *get_all(struct Map *map) {
  assert(map != NULL);
  kvpair_t *kv_array = map->kv_array;
  return kv_array;
}

symbolv_t *get_value_from_map(struct Map *map, symbol_t symbol) {
  struct KVPair *kv_array = map->kv_array;
  int i = 0;
  // move in array until an empty
  for (i = 0; i < map->size; i++) {
    if (kv_array[i].symbol != NULL) {
      symbol_t *curr = kv_array[i].symbol;
      if (curr->symbol_type == symbol.symbol_type &&
          !strcmp(curr->name, symbol.name)) {
        return kv_array[i].value;
      }
    }
  }
  return NULL;
}

bool map_contains(struct Map *map, symbol_t symbol) {
  return get_value_from_map(map, symbol) != NULL;
}

void add_to_map(struct Map *map, symbol_t symbol, symbolv_t value) {
  struct KVPair *kv_array = map->kv_array;
  struct KVPair *item = (struct KVPair *)malloc(sizeof(struct KVPair));

  item->symbol = malloc(sizeof(symbol));
  item->symbol->name = calloc(strlen(symbol.name), sizeof(char));
  item->symbol->symbol_type = symbol.symbol_type;
  strcpy(item->symbol->name, symbol.name);

  item->value = malloc(sizeof(symbolv_t));
  item->value->value_type = value.value_type;
  switch (value.value_type) {
  case UINT32T:
    item->value->uint_val = calloc(1, sizeof(uint32_t));
    *(item->value->uint_val) = *(value.uint_val);
    break;
  case STRING:
    item->value->str_val = calloc(strlen(value.str_val), sizeof(char));
    strcpy(item->value->str_val, value.str_val);
    break;
  case POINTER:
    item->value->ptr = malloc(sizeof(value.ptr));
    item->value->ptr = value.ptr;
    break;
  }

  if(map_contains(map, symbol)){
    for(int i = 0; i < map->size; i++){
      if(kv_array[i].symbol || kv_array[i].value){
        if(!cmp_symbols(*kv_array[i].symbol, symbol)){
          kv_array[i].value = malloc(sizeof(symbolv_t));
          kv_array[i].value->value_type = item->value->value_type;
          switch (item->value->value_type) {
            case UINT32T:
              kv_array[i].value->uint_val = item->value->uint_val;
              break;
            case STRING:
              kv_array[i].value->str_val = item->value->str_val;
              break;
            case POINTER:
              kv_array[i].value->ptr = item->value->ptr;
              break;
          }
          free(item->value);
          free(item->symbol);
          return;
        }
      }
    }
  }

  int i = 0;
  while(kv_array[i].symbol && i < map->size){
    i++;
  }
  kv_array[i] = *item;
  map->num_added++;
}

int get_num_added(struct Map *map) { return map->num_added; }

void free_map(struct Map *map) {
  assert(map != NULL);

  // Free the array
  struct KVPair *kv_array = map->kv_array;
  int i = 0;
  // move in array until an empty cell
  while (kv_array[i].symbol && i < map->size) {
    free(kv_array[i].symbol);
    free(kv_array[i].value);
    ++i;
  }
  // Now that array elems are free we can free array
  free(map->kv_array);
  free(map);
}

int cmp_symbols(symbol_t a, symbol_t b){
  int diff = (a.symbol_type - b.symbol_type);
  if(diff){
    return diff;
  }
  diff = strcmp(a.name, b.name);
  return diff;
}

bool map_remove_element(struct Map *map, symbol_t symbol){
  int i = 0;
  struct KVPair *kv_array = map->kv_array;
  while (i < map->size) {
    if(kv_array[i].symbol){
      if(!cmp_symbols(*kv_array[i].symbol, symbol)){
        kv_array[i].symbol = NULL;
        kv_array[i].value = NULL;
        map->num_added--;
        return true;
      }
    }
    ++i;
  }
  return false;
}

void print_map(struct Map *map) {
  struct KVPair *kv_array = map->kv_array;
  int i = 0;

  for (i = 0; i < map->size; i++) {
    if (kv_array[i].symbol != NULL){
      switch (kv_array[i].symbol->symbol_type){
        case LABEL:
          printf("Label : (");
          break;
        case CONSTANT:
          printf("Constant : (");
          break;
        case VARIABLE:
          printf("Variable : (");
          break;
        case ASSEMBLER_VAR:
          printf("Assembler Variable : (");
          break;
        case INSTR_EVAL:
          printf("Instruction Evaluation : (");
      }
      printf("%s, ", kv_array[i].symbol->name);
      switch (kv_array[i].value->value_type){
        case UINT32T:
          printf("%d) \n", *kv_array[i].value->uint_val);
          break;
        case STRING:
          printf("%s) \n", kv_array[i].value->str_val);
          break;
        case POINTER:
          printf("%p) \n", kv_array[i].value->ptr);
          break;
      }
    }
  }

  printf("\n");
}
