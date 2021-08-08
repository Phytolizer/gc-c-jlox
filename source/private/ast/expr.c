#include <gc.h>
#include <private/ast/expr.h>

#include "private/token.h"

static size_t IndentLevel = 0;

static void print_indent(void);

struct binary_expr* expr_new_binary(struct expr* left,
                                    struct token op,
                                    struct expr* right)
{
  struct binary_expr* expr = GC_MALLOC(sizeof(struct binary_expr));
  expr->base.type = EXPR_BINARY;
  expr->left = left;
  expr->op = op;
  expr->right = right;
  return expr;
}

struct grouping_expr* expr_new_grouping(struct expr* expression)
{
  struct grouping_expr* expr = GC_MALLOC(sizeof(struct grouping_expr));
  expr->base.type = EXPR_GROUPING;
  expr->expression = expression;
  return expr;
}

struct literal_expr* expr_new_literal(struct object value)
{
  struct literal_expr* expr = GC_MALLOC(sizeof(struct literal_expr));
  expr->base.type = EXPR_LITERAL;
  expr->value = value;
  return expr;
}

struct unary_expr* expr_new_unary(struct token op, struct expr* right)
{
  struct unary_expr* expr = GC_MALLOC(sizeof(struct unary_expr));
  expr->base.type = EXPR_UNARY;
  expr->op = op;
  expr->right = right;
  return expr;
}

void expr_print(struct expr* expr)
{
  switch (expr->type) {
    case EXPR_BINARY: {
      struct binary_expr* binary = (struct binary_expr*)expr;
      print_indent();
      printf("BINARY_EXPR {\n");
      ++IndentLevel;
      print_indent();
      printf(".left = ");
      expr_print(binary->left);
      printf(",\n");
      print_indent();
      printf(".operator = ");
      token_print(&binary->op);
      printf(",\n");
      print_indent();
      printf(".right = ");
      expr_print(binary->right);
      printf(",\n");
      --IndentLevel;
      print_indent();
      printf("}");
      break;
    }
    case EXPR_GROUPING: {
      struct grouping_expr* grouping = (struct grouping_expr*)expr;
      print_indent();
      printf("GROUPING_EXPR {\n");
      ++IndentLevel;
      print_indent();
      printf(".expression = ");
      expr_print(grouping->expression);
      printf(",\n");
      --IndentLevel;
      print_indent();
      printf("}");
      break;
    }
    case EXPR_LITERAL: {
      struct literal_expr* literal = (struct literal_expr*)expr;
      print_indent();
      printf("LITERAL_EXPR {\n");
      ++IndentLevel;
      print_indent();
      printf(".value = ");
      object_print(&literal->value);
      printf(",\n");
      --IndentLevel;
      print_indent();
      printf("}");
      break;
    }
    case EXPR_UNARY: {
      struct unary_expr* unary = (struct unary_expr*)expr;
      print_indent();
      printf("UNARY_EXPR {\n");
      ++IndentLevel;
      print_indent();
      printf(".op = ");
      token_print(&unary->op);
      printf(",\n");
      print_indent();
      printf(".right = ");
      expr_print(unary->right);
      printf(",\n");
      --IndentLevel;
      print_indent();
      printf("}");
      break;
    }
  }
}

static void print_indent(void)
{
  for (size_t i = 0; i < IndentLevel; ++i) {
    printf("  ");
  }
}
