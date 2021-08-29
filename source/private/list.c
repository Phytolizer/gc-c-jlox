#include "list.h"

#include <gc.h>

#define LIST_INITIAL_CAPACITY 8

void list_reserve_(struct list_unpacked_ list, long capacity)
{
  if (*list.capacity < capacity) {
    while (*list.capacity < capacity) {
      *list.capacity =
          (*list.capacity == 0) ? LIST_INITIAL_CAPACITY : *list.capacity * 2;
    }
    *list.pointer = GC_REALLOC(*list.pointer, *list.capacity * list.sizeof_t);
  }
}

void list_resize_(struct list_unpacked_ list, long length)
{
  list_reserve_(list, length);
  *list.length = length;
}
