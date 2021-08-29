#pragma once

#include <private/list.h>
#include <private/object.h>
#include <private/token_type.h>

struct token {
  enum token_type type;
  char* lexeme;
  struct object* literal;
  size_t line;
};

DECLARE_NAMED_LIST(token_list, struct token);

size_t token_print(const struct token* tok);
size_t token_fprint(FILE* f, const struct token* tok);
size_t token_snprint(char* s, size_t n, const struct token* tok);

struct token_list* token_list_new(void);
