#pragma once

#include <private/hash/table.h>
#include <private/object.h>
#include <private/runtime_error.h>
#include <private/token.h>

struct environment {
  struct environment* enclosing;
  struct hash_table* values;
};

enum environment_lookup_result_type
{
  ENVIRONMENT_LOOKUP_RESULT_OK,
  ENVIRONMENT_LOOKUP_RESULT_ERROR,
};

struct environment_lookup_result {
  enum environment_lookup_result_type type;
  union {
    struct object* o;
    struct runtime_error* e;
  } u;
};

#define ENVIRONMENT_LOOKUP_OK(obj) \
  (struct environment_lookup_result) \
  { \
    .type = ENVIRONMENT_LOOKUP_RESULT_OK, .u = {.o = (obj) } \
  }
#define ENVIRONMENT_LOOKUP_ERROR(err) \
  (struct environment_lookup_result) \
  { \
    .type = ENVIRONMENT_LOOKUP_RESULT_ERROR, .u = {.e = (err) } \
  }

#define ENVIRONMENT_LOOKUP_RESULT_IS_OK(res) \
  ((res)->type == ENVIRONMENT_LOOKUP_RESULT_OK)
#define ENVIRONMENT_LOOKUP_RESULT_IS_ERROR(res) \
  ((res)->type == ENVIRONMENT_LOOKUP_RESULT_ERROR)

#define ENVIRONMENT_LOOKUP_RESULT_GET_OK(res) ((res)->u.o)
#define ENVIRONMENT_LOOKUP_RESULT_GET_ERROR(res) ((res)->u.e)

struct environment* environment_new(void);
struct environment* environment_new_enclosed(struct environment* enclosing);
void environment_define(struct environment* environment,
                        const char* name,
                        struct object* value);
struct environment_lookup_result environment_get(
    struct environment* environment, struct token* name);
struct runtime_error* environment_assign(struct environment* environment,
                                         struct token* name,
                                         struct object* value);
