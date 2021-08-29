#pragma once

#include <assert.h>
#include <private/token.h>

enum expr_type
{
  EXPR_ASSIGN,
  EXPR_BINARY,
  EXPR_GROUPING,
  EXPR_LITERAL,
  EXPR_LOGICAL,
  EXPR_UNARY,
  EXPR_VARIABLE,
};

struct expr {
  enum expr_type type;
};

struct assign_expr {
  struct expr base;
  struct token name;
  struct expr* value;
};

struct binary_expr {
  struct expr base;
  struct expr* left;
  struct token op;
  struct expr* right;
};

struct grouping_expr {
  struct expr base;
  struct expr* expression;
};

struct literal_expr {
  struct expr base;
  struct object* value;
};

struct logical_expr {
  struct expr base;
  struct expr* left;
  struct token op;
  struct expr* right;
};

struct unary_expr {
  struct expr base;
  struct token op;
  struct expr* right;
};

struct variable_expr {
  struct expr base;
  struct token name;
};

struct assign_expr* expr_new_assign(struct token name, struct expr* value);
struct binary_expr* expr_new_binary(struct expr* left,
                                    struct token op,
                                    struct expr* right);
struct grouping_expr* expr_new_grouping(struct expr* expression);
struct literal_expr* expr_new_literal(struct object* value);
struct logical_expr* expr_new_logical(struct expr* left,
                                      struct token op,
                                      struct expr* right);
struct unary_expr* expr_new_unary(struct token op, struct expr* right);
struct variable_expr* expr_new_variable(struct token name);

#define EXPR_DECLARE_ACCEPT_FOR(result_type, visitor_type) \
  result_type expr_accept_##visitor_type(struct expr* expr, \
                                         struct visitor_type* v)

#define EXPR_DEFINE_ACCEPT_FOR(result_type, visitor_type) \
  EXPR_DECLARE_ACCEPT_FOR(result_type, visitor_type) \
  { \
    switch (expr->type) { \
      case EXPR_ASSIGN: \
        return visitor_type##_visit_assign_expr(v, (struct assign_expr*)expr); \
      case EXPR_BINARY: \
        return visitor_type##_visit_binary_expr(v, (struct binary_expr*)expr); \
      case EXPR_GROUPING: \
        return visitor_type##_visit_grouping_expr( \
            v, (struct grouping_expr*)expr); \
      case EXPR_LITERAL: \
        return visitor_type##_visit_literal_expr(v, \
                                                 (struct literal_expr*)expr); \
      case EXPR_LOGICAL: \
        return visitor_type##_visit_logical_expr(v, \
                                                 (struct logical_expr*)expr); \
      case EXPR_UNARY: \
        return visitor_type##_visit_unary_expr(v, (struct unary_expr*)expr); \
      case EXPR_VARIABLE: \
        return visitor_type##_visit_variable_expr( \
            v, (struct variable_expr*)expr); \
    } \
    assert(false && "Entered unreachable code"); \
  }
