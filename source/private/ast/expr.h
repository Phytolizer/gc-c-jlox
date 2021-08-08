#pragma once

#include <private/token.h>

enum expr_type
{
  EXPR_BINARY,
  EXPR_GROUPING,
  EXPR_LITERAL,
  EXPR_UNARY,
};

struct expr {
  enum expr_type type;
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
  struct object value;
};

struct unary_expr {
  struct expr base;
  struct token op;
  struct expr* right;
};

struct binary_expr* expr_new_binary(struct expr* left,
                                    struct token op,
                                    struct expr* right);
struct grouping_expr* expr_new_grouping(struct expr* expression);
struct literal_expr* expr_new_literal(struct object value);
struct unary_expr* expr_new_unary(struct token op, struct expr* right);

#define EXPR_DECLARE_ACCEPT_FOR(result_type, visitor_type) \
  result_type expr_accept_##visitor_type(struct expr* expr, \
                                         struct visitor_type* v)

#define EXPR_DEFINE_ACCEPT_FOR(result_type, visitor_type) \
  EXPR_DECLARE_ACCEPT_FOR(result_type, visitor_type) \
  { \
    switch (expr->type) { \
      case EXPR_BINARY: \
        return visitor_type##_visit_binary_expr(v, (struct binary_expr*)expr); \
      case EXPR_GROUPING: \
        return visitor_type##_visit_grouping_expr( \
            v, (struct grouping_expr*)expr); \
      case EXPR_LITERAL: \
        return visitor_type##_visit_literal_expr(v, \
                                                 (struct literal_expr*)expr); \
      case EXPR_UNARY: \
        return visitor_type##_visit_unary_expr(v, (struct unary_expr*)expr); \
    } \
  }
