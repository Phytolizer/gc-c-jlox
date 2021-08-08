#include <gc.h>
#include <private/ast/stmt_list.h>

#define STMT_LIST_INITIAL_CAPACITY 8

struct stmt_list* stmt_list_new(void)
{
  struct stmt_list* list = GC_MALLOC(sizeof(struct stmt_list));
  list->data = NULL;
  list->length = 0;
  list->capacity = 0;
  return list;
}

void stmt_list_push(struct stmt_list* list, struct stmt* stmt)
{
  if (list->length == list->capacity) {
    if (list->capacity == 0) {
      list->capacity = STMT_LIST_INITIAL_CAPACITY;
    } else {
      list->capacity *= 2;
    }
    list->data = GC_REALLOC(list->data, list->capacity * sizeof(struct stmt*));
  }
  list->data[list->length] = stmt;
  ++list->length;
}
