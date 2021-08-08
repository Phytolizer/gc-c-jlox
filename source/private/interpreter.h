#pragma once

#include <private/ast/expr.h>
#include <private/ast/stmt.h>
#include <private/ast/stmt_list.h>
#include <private/object.h>
#include <private/runtime_error.h>

struct interpreter {
};

EXPR_DECLARE_ACCEPT_FOR(struct interpret_result, interpreter);
STMT_DECLARE_ACCEPT_FOR(struct runtime_error*, interpreter);

void interpret(struct interpreter* interpreter, struct stmt_list* statements);
