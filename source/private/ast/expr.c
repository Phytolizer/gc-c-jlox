#include <gc.h>
#include <private/ast/expr.h>

#include "private/token.h"

struct assign_expr* expr_new_assign(struct token name, struct expr* value)
{
  struct assign_expr* expr = GC_MALLOC(sizeof(struct assign_expr));
  expr->base.type = EXPR_ASSIGN;
  expr->name = name;
  expr->value = value;
  return expr;
}

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

struct call_expr* expr_new_call(struct expr* callee,
                                struct token paren,
                                struct expr_list* arguments)
{
  struct call_expr* expr = GC_MALLOC(sizeof(struct call_expr));
  expr->base.type = EXPR_CALL;
  expr->callee = callee;
  expr->paren = paren;
  expr->arguments = arguments;
  return expr;
}

struct grouping_expr* expr_new_grouping(struct expr* expression)
{
  struct grouping_expr* expr = GC_MALLOC(sizeof(struct grouping_expr));
  expr->base.type = EXPR_GROUPING;
  expr->expression = expression;
  return expr;
}

struct literal_expr* expr_new_literal(struct object* value)
{
  struct literal_expr* expr = GC_MALLOC(sizeof(struct literal_expr));
  expr->base.type = EXPR_LITERAL;
  expr->value = value;
  return expr;
}

struct logical_expr* expr_new_logical(struct expr* left,
                                      struct token op,
                                      struct expr* right)
{
  struct logical_expr* expr = GC_MALLOC(sizeof(struct logical_expr));
  expr->base.type = EXPR_LOGICAL;
  expr->left = left;
  expr->op = op;
  expr->right = right;
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

struct variable_expr* expr_new_variable(struct token name)
{
  struct variable_expr* expr = GC_MALLOC(sizeof(struct variable_expr));
  expr->base.type = EXPR_VARIABLE;
  expr->name = name;
  return expr;
}
