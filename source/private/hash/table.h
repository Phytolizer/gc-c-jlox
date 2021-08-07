#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct hash_bucket {
  char* key;
  int value;
};

typedef uint64_t (*hash_function)(const void* data, size_t len);

struct hash_table {
  struct hash_bucket* data;
  hash_function function;
  size_t len;
  size_t cap;
};

struct hash_table* hash_table_new(hash_function function);
void hash_table_insert(struct hash_table* table, const char* key_begin, int value);
bool hash_table_contains(struct hash_table* table, const char* key_begin, size_t key_len);
int* hash_table_try_get(struct hash_table* table, const char* key_begin, size_t key_len);
