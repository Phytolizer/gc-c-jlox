#pragma once

#include <private/ast/expr.h>
#include <private/ast/stmt.h>
#include <private/environment.h>
#include <private/object.h>
#include <private/runtime_error.h>
#include <time.h>

struct interpreter {
  struct environment* globals;
  struct environment* environment;
  time_t init_time;
};

EXPR_DECLARE_ACCEPT_FOR(struct interpret_result, interpreter);
STMT_DECLARE_ACCEPT_FOR(struct execution_result, interpreter);

struct interpreter* interpreter_new(void);
void interpret(struct interpreter* interpreter, struct stmt_list* statements);
