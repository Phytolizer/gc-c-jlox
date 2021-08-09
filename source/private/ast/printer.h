#pragma once

#include <private/ast/expr.h>

// empty struct
struct ast_printer {
};

void print_ast(struct expr* expr);

EXPR_DECLARE_ACCEPT_FOR(char*, ast_printer);
