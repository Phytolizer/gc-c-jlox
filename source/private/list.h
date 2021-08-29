#pragma once

#include <stdbool.h>
#include <stddef.h>

#define LIST(T) \
  struct { \
    T* pointer; \
    long length; \
    long capacity; \
  }

#define DECLARE_NAMED_LIST(Name, T) \
  struct Name { \
    T* pointer; \
    long length; \
    long capacity; \
  }

struct list_unpacked_ {
  char** pointer;
  size_t sizeof_t;
  long* length;
  long* capacity;
};

#define LIST_UNPACK_(L) \
  (struct list_unpacked_) \
  { \
    .pointer = (char**)&(L)->pointer, .sizeof_t = sizeof(*(L)->pointer), \
    .length = &(L)->length, .capacity = &(L)->capacity \
  }

void list_reserve_(struct list_unpacked_ list, long capacity);
void list_resize_(struct list_unpacked_ list, long length);

#define LIST_RESERVE(L, Capacity) list_reserve_(LIST_UNPACK_(L), Capacity)
#define LIST_RESIZE(L, Length) list_resize_(LIST_UNPACK_(L), Length)
#define LIST_PUSH(L, Elem) \
  do { \
    LIST_RESERVE(L, (L)->length + 1); \
    (L)->pointer[(L)->length] = (Elem); \
    ++(L)->length; \
  } while (false)
