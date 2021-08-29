#include <gc.h>
#include <private/environment.h>
#include <private/hash/fnv.h>
#include <private/hash/table.h>
#include <string.h>

#include "private/strutils.h"

struct environment* environment_new(void)
{
  struct environment* environment = GC_MALLOC(sizeof(struct environment));
  environment->enclosing = NULL;
  environment->values = hash_table_new(hash_fnv1a);
  return environment;
}

struct environment* environment_new_enclosed(struct environment* enclosing)
{
  struct environment* environment = GC_MALLOC(sizeof(struct environment));
  environment->enclosing = enclosing;
  environment->values = hash_table_new(hash_fnv1a);
  return environment;
}

void environment_define(struct environment* environment,
                        const char* name,
                        struct object* value)
{
  hash_table_insert(environment->values, name, value);
}

struct environment_lookup_result environment_get(
    struct environment* environment, struct token* name)
{
  struct object** result = (struct object**)hash_table_try_get(
      environment->values, name->lexeme, strlen(name->lexeme));
  if (result) {
    return ENVIRONMENT_LOOKUP_OK(*result);
  }

  if (environment->enclosing != NULL) {
    return environment_get(environment->enclosing, name);
  }

  return ENVIRONMENT_LOOKUP_ERROR(runtime_error_new(
      name, alloc_printf("Undefined variable '%s'.", name->lexeme)));
}

struct runtime_error* environment_assign(struct environment* environment,
                                         struct token* name,
                                         struct object* value)
{
  struct object** loc = (struct object**)hash_table_try_get(
      environment->values, name->lexeme, strlen(name->lexeme));
  if (loc) {
    *loc = value;
    return NULL;
  }

  if (environment->enclosing != NULL) {
    return environment_assign(environment->enclosing, name, value);
  }

  return runtime_error_new(
      name, alloc_printf("Undefined variable '%s'.", name->lexeme));
}
