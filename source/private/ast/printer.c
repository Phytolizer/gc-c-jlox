#include <gc.h>
#include <private/ast/printer.h>
#include <private/strutils.h>
#include <stdio.h>
#include <string.h>

#define TOKEN_PREFIX_LEN 8

static char* dump_token(const struct token* token);

void print_ast(struct expr* expr)
{
  struct ast_printer printer;
  printf("%s\n", expr_accept_ast_printer(expr, &printer));
}

char* ast_printer_visit_binary_expr(struct ast_printer* printer,
                                    struct binary_expr* expr)
{
  char* left = expr_accept_ast_printer(expr->left, printer);
  char* op = dump_token(&expr->op);
  char* right = expr_accept_ast_printer(expr->right, printer);

  return alloc_printf("(%s %s %s)", op, left, right);
}

char* ast_printer_visit_grouping_expr(struct ast_printer* printer,
                                      struct grouping_expr* expr)
{
  char* expression = expr_accept_ast_printer(expr->expression, printer);
  return alloc_printf("(%s)", expression);
}

char* ast_printer_visit_literal_expr(struct ast_printer* printer,
                                     struct literal_expr* expr)
{
  (void)printer;
  size_t len = object_snprint(NULL, 0, &expr->value);
  char* str = GC_MALLOC(len + 1);
  object_snprint(str, len + 1, &expr->value);
  return str;
}

char* ast_printer_visit_unary_expr(struct ast_printer* printer,
                                   struct unary_expr* expr)
{
  char* op = dump_token(&expr->op);
  char* right = expr_accept_ast_printer(expr->right, printer);
  return alloc_printf("(%s%s)", op, right);
}

static char* dump_token(const struct token* token)
{
  size_t toklen = token_snprint(NULL, 0, token);
  // including null terminator in len because of sizeof()
  size_t len = toklen + sizeof("Token {  }");
  char* str = GC_MALLOC(len);
  char* p = str;
  p += sprintf(p, "Token { ");
  p += token_snprint(p, toklen, token);
  sprintf(p, " }");
  return str;
}

EXPR_DEFINE_ACCEPT_FOR(char*, ast_printer)
