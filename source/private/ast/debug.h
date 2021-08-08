#pragma once

#include <private/ast/expr.h>
#include <private/ast/stmt.h>
#include <stddef.h>

extern size_t AstDebugIndentLevel;

void ast_debug_print_indent(void);
void expr_debug(struct expr* expr);
