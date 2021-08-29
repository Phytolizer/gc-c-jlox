#include <gc.h>
#include <private/ast/printer.h>
#include <private/strutils.h>
#include <stdio.h>
#include <string.h>

#define TOKEN_PREFIX_LEN 8

static char* ast_printer_visit_assign_expr(struct ast_printer* printer,
                                           struct assign_expr* expr);
static char* ast_printer_visit_binary_expr(struct ast_printer* printer,
                                           struct binary_expr* expr);
static char* ast_printer_visit_grouping_expr(struct ast_printer* printer,
                                             struct grouping_expr* expr);
static char* ast_printer_visit_literal_expr(struct ast_printer* printer,
                                            struct literal_expr* expr);
static char* ast_printer_visit_logical_expr(struct ast_printer* printer,
                                            struct logical_expr* expr);
static char* ast_printer_visit_unary_expr(struct ast_printer* printer,
                                          struct unary_expr* expr);
static char* ast_printer_visit_variable_expr(struct ast_printer* printer,
                                             struct variable_expr* expr);

void print_ast(struct expr* expr)
{
  struct ast_printer printer;
  printf("%s\n", expr_accept_ast_printer(expr, &printer));
}

static char* ast_printer_visit_assign_expr(struct ast_printer* printer,
                                           struct assign_expr* expr)
{
  char* value = expr_accept_ast_printer(expr->value, printer);
  return alloc_printf("(set %s %s)", expr->name.lexeme, value);
}

static char* ast_printer_visit_binary_expr(struct ast_printer* printer,
                                           struct binary_expr* expr)
{
  char* left = expr_accept_ast_printer(expr->left, printer);
  char* op = expr->op.lexeme;
  char* right = expr_accept_ast_printer(expr->right, printer);

  return alloc_printf("(%s %s %s)", op, left, right);
}

static char* ast_printer_visit_grouping_expr(struct ast_printer* printer,
                                             struct grouping_expr* expr)
{
  char* expression = expr_accept_ast_printer(expr->expression, printer);
  return alloc_printf("(%s)", expression);
}

static char* ast_printer_visit_literal_expr(struct ast_printer* printer,
                                            struct literal_expr* expr)
{
  (void)printer;
  size_t len = object_snprint(NULL, 0, expr->value);
  char* str = GC_MALLOC(len + 1);
  object_snprint(str, len + 1, expr->value);
  return str;
}

static char* ast_printer_visit_logical_expr(struct ast_printer* printer,
                                            struct logical_expr* expr)
{
  char* left = expr_accept_ast_printer(expr->left, printer);
  char* right = expr_accept_ast_printer(expr->right, printer);

  if (expr->op.type == TOKEN_OR) {
    return alloc_printf("(or %s %s)", left, right);
  }
  return alloc_printf("(and %s %s)", left, right);
}

static char* ast_printer_visit_unary_expr(struct ast_printer* printer,
                                          struct unary_expr* expr)
{
  const char* op = expr->op.lexeme;
  char* right = expr_accept_ast_printer(expr->right, printer);
  return alloc_printf("(%s%s)", op, right);
}

static char* ast_printer_visit_variable_expr(struct ast_printer* printer,
                                             struct variable_expr* expr)
{
  (void)printer;
  return expr->name.lexeme;
}

EXPR_DEFINE_ACCEPT_FOR(char*, ast_printer);
