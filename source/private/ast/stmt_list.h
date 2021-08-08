#pragma once

#include <private/ast/stmt.h>
#include <stddef.h>

struct stmt_list {
  struct stmt** data;
  size_t length;
  size_t capacity;
};

struct stmt_list* stmt_list_new(void);
void stmt_list_push(struct stmt_list* list, struct stmt* stmt);
