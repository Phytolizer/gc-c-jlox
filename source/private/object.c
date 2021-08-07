#include <private/object.h>

void object_print(struct object* obj)
{
  object_fprint(stdout, obj);
}

void object_fprint(FILE* f, struct object* obj)
{
  switch (obj->type) {
    case OBJECT_TYPE_STRING:
      fprintf(f, "%s", OBJECT_AS_STRING(obj));
      break;
    case OBJECT_TYPE_NUMBER:
      fprintf(f, "%lg", OBJECT_AS_NUMBER(obj));
      break;
    case OBJECT_TYPE_NULL:
      fprintf(f, "NULL");
      break;
  }
}
