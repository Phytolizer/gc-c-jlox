#pragma once

#include <stdio.h>

enum object_type
{
  OBJECT_TYPE_STRING,
  OBJECT_TYPE_NUMBER,
  OBJECT_TYPE_NULL,
};

struct object {
  enum object_type type;
  union object_value {
    char* s;
    double num;
  } value;
};

#define OBJECT_INT(val) \
  (struct object) \
  { \
    .type = OBJECT_TYPE_INT, .value = {.i = (val)}, \
  }

#define OBJECT_STRING(val) \
  (struct object) \
  { \
    .type = OBJECT_TYPE_STRING, .value = {.s = (val)}, \
  }

#define OBJECT_NUMBER(val) \
  (struct object) \
  { \
    .type = OBJECT_TYPE_NUMBER, .value = {.num = (val) } \
  }

#define OBJECT_NULL() \
  (struct object) \
  { \
    .type = OBJECT_TYPE_NULL \
  }

#define OBJECT_IS_NUMBER(obj) ((obj)->type == OBJECT_TYPE_NUMBER)
#define OBJECT_IS_STRING(obj) ((obj)->type == OBJECT_TYPE_STRING)
#define OBJECT_IS_NULL(obj) ((obj)->type == OBJECT_TYPE_NULL)

#define OBJECT_AS_NUMBER(obj) (obj)->value.num
#define OBJECT_AS_STRING(obj) (obj)->value.s

size_t object_print(const struct object* obj);
size_t object_fprint(FILE* f, const struct object* obj);
size_t object_snprint(char* s, size_t n, const struct object* obj);
