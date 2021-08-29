#include <gc.h>
#include <private/ast/printer.h>
#include <private/list.h>
#include <private/strutils.h>
#include <stdio.h>
#include <string.h>

#define TOKEN_PREFIX_LEN 8

DECLARE_NAMED_LIST(string_list, char*);

static char* print_expr_list(struct ast_printer* printer,
                             struct expr_list* list);

static char* ast_printer_visit_assign_expr(struct ast_printer* printer,
                                           struct assign_expr* expr);
static char* ast_printer_visit_binary_expr(struct ast_printer* printer,
                                           struct binary_expr* expr);
static char* ast_printer_visit_call_expr(struct ast_printer* printer,
                                         struct call_expr* expr);
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

static char* print_expr_list(struct ast_printer* printer,
                             struct expr_list* list)
{
  // "[]\0"
  size_t len = 3;
  struct string_list* strings = GC_MALLOC(sizeof(struct string_list));
  LIST_INIT(strings);
  for (long i = 0; i < list->length; i++) {
    LIST_PUSH(strings, expr_accept_ast_printer(list->pointer[i], printer));
    len += strlen(strings->pointer[i]);
    // + ", "
    if (i < list->length - 1) {
      len += 2;
    }
  }
  char* result = GC_MALLOC(len);
  char* cursor = result;
  strcpy(cursor, "[");
  cursor += 1;
  for (long i = 0; i < list->length; i++) {
    strcpy(cursor, strings->pointer[i]);
    cursor += strlen(strings->pointer[i]);
    if (i < list->length - 1) {
      strcpy(cursor, ", ");
      cursor += 2;
    }
  }
  strcpy(cursor, "]");
  return result;
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

static char* ast_printer_visit_call_expr(struct ast_printer* printer,
                                         struct call_expr* expr)
{
  char* callee = expr_accept_ast_printer(expr->callee, printer);
  char* arguments = print_expr_list(printer, expr->arguments);
  return alloc_printf("(invoke %s %s)", callee, arguments);
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

  return alloc_printf(
      "(%s %s %s)", expr->op.type == TOKEN_OR ? "or" : "and", left, right);
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

EXPR_DEFINE_ACCEPT_FOR(char*, ast_printer)
