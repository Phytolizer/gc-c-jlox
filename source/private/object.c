#include <gc.h>
#include <private/assertions.h>
#include <private/object.h>
#include <stdarg.h>
#include <stdbool.h>

#define OBJECT_PRINT(prefix, p, n, obj) \
  do { \
    switch ((obj)->type) { \
      case OBJECT_TYPE_STRING: \
        return prefix##nprintf((p), (n), "%s", OBJECT_AS_STRING(obj)); \
      case OBJECT_TYPE_NUMBER: \
        return prefix##nprintf((p), (n), "%lg", OBJECT_AS_NUMBER(obj)); \
      case OBJECT_TYPE_BOOL: \
        return prefix##nprintf( \
            (p), (n), "%s", OBJECT_AS_BOOL(obj) ? "true" : "false"); \
      case OBJECT_TYPE_NULL: \
        return prefix##nprintf((p), (n), "NULL"); \
      case OBJECT_TYPE_NATIVE_FUNCTION: \
        return prefix##nprintf((p), (n), "<native function>"); \
    } \
  } while (false)

struct my_printer {
  size_t (*write)(struct my_printer* printer, const struct object* obj);
};

struct my_file_printer {
  struct my_printer base;
  FILE* fp;
};

struct my_string_printer {
  struct my_printer base;
  char* str;
  size_t len;
};

static struct my_file_printer* file_printer_new(FILE* fp);
static struct my_string_printer* string_printer_new(char* str, size_t len);
static size_t object_print_to_file(struct my_printer* printer,
                                   const struct object* obj);
static size_t object_print_to_string(struct my_printer* printer,
                                     const struct object* obj);
static size_t fnprintf(FILE* fp, size_t _n, const char* format, ...)
    __attribute__((format(printf, 3, 4)));

struct object* object_new_string(char* value)
{
  struct object* obj = GC_MALLOC(sizeof(struct object));
  obj->type = OBJECT_TYPE_STRING;
  obj->value.s = value;
  return obj;
}

struct object* object_new_number(double value)
{
  struct object* obj = GC_MALLOC(sizeof(struct object));
  obj->type = OBJECT_TYPE_NUMBER;
  obj->value.d = value;
  return obj;
}

struct object* object_new_bool(bool value)
{
  struct object* obj = GC_MALLOC(sizeof(struct object));
  obj->type = OBJECT_TYPE_BOOL;
  obj->value.b = value;
  return obj;
}

struct object* object_new_null(void)
{
  struct object* obj = GC_MALLOC(sizeof(struct object));
  obj->type = OBJECT_TYPE_NULL;
  return obj;
}

struct object* object_new_native_function(
    long arity, struct object* (*value)(struct interpreter*, struct object_list*))
{
  struct object* obj = GC_MALLOC(sizeof(struct object));
  obj->type = OBJECT_TYPE_NATIVE_FUNCTION;
  obj->value.nf.func = value;
  obj->value.nf.arity = arity;
  return obj;
}

long object_arity(struct object* obj)
{
  switch (obj->type) {
    case OBJECT_TYPE_NATIVE_FUNCTION:
      return OBJECT_AS_NATIVE_FUNCTION(obj).arity;
    default:
      ASSERT_UNREACHABLE();
  }
}

size_t object_print(const struct object* obj)
{
  return object_fprint(stdout, obj);
}

size_t object_fprint(FILE* f, const struct object* obj)
{
  return object_print_to_file((struct my_printer*)file_printer_new(f), obj);
}

size_t object_snprint(char* s, size_t n, const struct object* obj)
{
  return object_print_to_string((struct my_printer*)string_printer_new(s, n),
                                obj);
}

static struct my_file_printer* file_printer_new(FILE* fp)
{
  struct my_file_printer* handle = GC_MALLOC(sizeof(struct my_file_printer));
  handle->fp = fp;
  handle->base.write = object_print_to_file;
  return handle;
}

static struct my_string_printer* string_printer_new(char* str, size_t len)
{
  struct my_string_printer* handle =
      GC_MALLOC(sizeof(struct my_string_printer));
  handle->str = str;
  handle->len = len;
  return handle;
}

static size_t object_print_to_file(struct my_printer* printer,
                                   const struct object* obj)
{
  struct my_file_printer* handle = (struct my_file_printer*)printer;
  OBJECT_PRINT(f, handle->fp, 0, obj);
}

static size_t object_print_to_string(struct my_printer* printer,
                                     const struct object* obj)
{
  struct my_string_printer* handle = (struct my_string_printer*)printer;
  OBJECT_PRINT(s, handle->str, handle->len, obj);
}

static size_t fnprintf(FILE* fp, size_t _n, const char* format, ...)
{
  (void)_n;
  va_list args;
  va_start(args, format);
  size_t value = vfprintf(fp, format, args);
  va_end(args);
  return value;
}
