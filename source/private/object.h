#pragma once

#include <private/list.h>
#include <stdbool.h>
#include <stdio.h>

enum object_type
{
  OBJECT_TYPE_STRING,
  OBJECT_TYPE_NUMBER,
  OBJECT_TYPE_BOOL,
  OBJECT_TYPE_NULL,
  OBJECT_TYPE_NATIVE_FUNCTION,
};

struct object_list;

struct native_function {
  long arity;
  struct object* (*func)(struct object_list*);
};

struct object {
  enum object_type type;
  union object_value {
    char* s;
    double d;
    bool b;
    struct native_function nf;
  } value;
};

DECLARE_NAMED_LIST(object_list, struct object*);

struct object* object_new_string(char* value);
struct object* object_new_number(double value);
struct object* object_new_bool(bool value);
struct object* object_new_null(void);
struct object* object_new_native_function(
    long arity, struct object* (*value)(struct object_list*));

long object_arity(struct object* obj);

#define OBJECT_STRING(val) ((struct object*)object_new_string(val))
#define OBJECT_NUMBER(val) ((struct object*)object_new_number(val))
#define OBJECT_BOOL(val) ((struct object*)object_new_bool(val))
#define OBJECT_NULL() ((struct object*)object_new_null())
#define OBJECT_NATIVE_FUNCTION(arity, val) \
  ((struct object*)object_new_native_function(arity, val))

#define OBJECT_IS_NUMBER(obj) ((obj)->type == OBJECT_TYPE_NUMBER)
#define OBJECT_IS_STRING(obj) ((obj)->type == OBJECT_TYPE_STRING)
#define OBJECT_IS_BOOL(obj) ((obj)->type == OBJECT_TYPE_BOOL)
#define OBJECT_IS_NULL(obj) ((obj)->type == OBJECT_TYPE_NULL)
#define OBJECT_IS_CALLABLE(obj) ((obj)->type == OBJECT_TYPE_NATIVE_FUNCTION)

#define OBJECT_AS_NUMBER(obj) (obj)->value.d
#define OBJECT_AS_STRING(obj) (obj)->value.s
#define OBJECT_AS_BOOL(obj) (obj)->value.b
#define OBJECT_AS_NATIVE_FUNCTION(obj) (obj)->value.nf

size_t object_print(const struct object* obj);
size_t object_fprint(FILE* f, const struct object* obj);
size_t object_snprint(char* s, size_t n, const struct object* obj);
