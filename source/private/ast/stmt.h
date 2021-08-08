#pragma once

enum stmt_type
{
  STMT_EXPRESSION,
  STMT_PRINT,
};

struct stmt {
  enum stmt_type type;
};

struct expression_stmt {
  struct stmt base;
  struct expr* expression;
};

struct print_stmt {
  struct stmt base;
  struct expr* expression;
};

struct expression_stmt* stmt_new_expression(struct expr* expression);
struct print_stmt* stmt_new_print(struct expr* expression);

#define STMT_DECLARE_ACCEPT_FOR(result_type, visitor_type) \
  result_type stmt_accept_##visitor_type(struct stmt* stmt, \
                                         struct visitor_type* visitor)

#define STMT_DEFINE_ACCEPT_FOR(result_type, visitor_type) \
  STMT_DECLARE_ACCEPT_FOR(result_type, visitor_type) \
  { \
    switch (stmt->type) { \
      case STMT_EXPRESSION: \
        return visitor_type##_visit_expression_stmt(visitor, stmt); \
      case STMT_PRINT: \
        return visitor_type##_visit_print_stmt(visitor, stmt); \
    } \
  }
