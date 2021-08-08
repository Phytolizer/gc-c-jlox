#pragma once

#include <private/ast/expr.h>

// empty struct
struct ast_printer {
};

void print_ast(struct expr* expr);

char* ast_printer_visit_binary_expr(struct ast_printer* printer,
                                    struct binary_expr* expr);
char* ast_printer_visit_grouping_expr(struct ast_printer* printer,
                                      struct grouping_expr* expr);
char* ast_printer_visit_literal_expr(struct ast_printer* printer,
                                     struct literal_expr* expr);
char* ast_printer_visit_unary_expr(struct ast_printer* printer,
                                   struct unary_expr* expr);

EXPR_DECLARE_ACCEPT_FOR(char*, ast_printer);
