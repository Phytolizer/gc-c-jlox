#include <gc.h>
#include <private/hash/table.h>
#include <string.h>

#define INITIAL_CAPACITY 8
#define MAX_LOAD 0.75

static void rehash(struct hash_table* table);
static void insert_raw(struct hash_bucket* buckets,
                       size_t cap,
                       uint64_t hash,
                       char* key,
                       void* value);

struct hash_table* hash_table_new(hash_function function)
{
  struct hash_table* table = GC_MALLOC(sizeof(struct hash_table));
  table->data = NULL;
  table->len = 0;
  table->cap = 0;
  table->function = function;
  return table;
}

void hash_table_insert(struct hash_table* table, const char* key, void* value)
{
  if (table->len == 0 || (double)table->len / (double)table->cap > MAX_LOAD) {
    rehash(table);
  }
  insert_raw(table->data,
             table->cap,
             table->function(key, strlen(key)),
             GC_STRDUP(key),
             value);
  ++table->len;
}

bool hash_table_contains(struct hash_table* table,
                         const char* key_begin,
                         size_t key_len)
{
  return hash_table_try_get(table, key_begin, key_len) != NULL;
}

void** hash_table_try_get(struct hash_table* table,
                          const char* key_begin,
                          size_t key_len)
{
  if (table->len == 0) {
    return NULL;
  }
  uint64_t hash = table->function(key_begin, key_len) % table->cap;
  while (table->data[hash].key) {
    if (key_len == strlen(table->data[hash].key)
        && strncmp(table->data[hash].key, key_begin, key_len) == 0)
    {
      return &table->data[hash].value;
    }
    hash = (hash + 1) % table->cap;
  }
  return NULL;
}

static void rehash(struct hash_table* table)
{
  size_t oldcap = table->cap;
  if (table->cap == 0) {
    table->cap = INITIAL_CAPACITY;
  } else {
    table->cap *= 2;
  }
  struct hash_bucket* newdata =
      GC_MALLOC(table->cap * sizeof(struct hash_bucket));
  for (size_t i = 0; i < table->cap; ++i) {
    newdata[i].key = NULL;
  }
  for (size_t i = 0; i < oldcap; ++i) {
    if (table->data[i].key) {
      uint64_t newhash =
          table->function(table->data[i].key, strlen(table->data[i].key));
      insert_raw(newdata,
                 table->cap,
                 newhash,
                 table->data[i].key,
                 table->data[i].value);
    }
  }
  table->data = newdata;
}

static void insert_raw(struct hash_bucket* buckets,
                       size_t cap,
                       uint64_t hash,
                       char* key,
                       void* value)
{
  hash %= cap;
  while (buckets[hash].key) {
    hash = (hash + 1) % cap;
  }
  buckets[hash].key = GC_STRDUP(key);
  buckets[hash].value = value;
}
