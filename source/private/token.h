#pragma once

#include <private/object.h>
#include <private/token_type.h>

struct token {
  enum token_type type;
  char* lexeme;
  struct object literal;
  size_t line;
};

struct token_list {
  struct token* data;
  size_t length;
  size_t capacity;
};

void token_print(struct token* tok);
void token_fprint(FILE* f, struct token* tok);

struct token_list* token_list_new(void);
void token_list_push(struct token_list* list, struct token tok);
