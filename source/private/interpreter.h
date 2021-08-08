#pragma once

#include <private/ast/expr.h>
#include <private/object.h>

struct interpreter {
};

EXPR_DECLARE_ACCEPT_FOR(struct interpret_result, interpreter);

void interpret(struct interpreter* interpreter, struct expr* expression);
