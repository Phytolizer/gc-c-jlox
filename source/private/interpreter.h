#pragma once

#include <private/ast/expr.h>
#include <private/ast/stmt.h>
#include <private/environment.h>
#include <private/object.h>
#include <private/runtime_error.h>

struct interpreter {
  struct environment* globals;
  struct environment* environment;
};

EXPR_DECLARE_ACCEPT_FOR(struct interpret_result, interpreter);
STMT_DECLARE_ACCEPT_FOR(struct runtime_error*, interpreter);

struct interpreter* interpreter_new(void);
void interpret(struct interpreter* interpreter, struct stmt_list* statements);
