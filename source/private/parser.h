#pragma once

#include <private/ast/expr.h>
#include <private/ast/stmt.h>
#include <private/token.h>

struct parser {
  struct token_list* tokens;
  size_t current;
};

struct parser* parser_new(struct token_list* tokens);
struct expr* parser_parse(struct parser* parser);
