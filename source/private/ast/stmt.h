#pragma once

#include <assert.h>
#include <private/list.h>
#include <private/token.h>
#include <stdbool.h>

enum stmt_type
{
  STMT_BLOCK,
  STMT_EXPRESSION,
  STMT_IF,
  STMT_PRINT,
  STMT_VAR,
};

struct stmt {
  enum stmt_type type;
};

DECLARE_NAMED_LIST(stmt_list, struct stmt*);

struct stmt_list* stmt_list_new(void);

struct block_stmt {
  struct stmt base;
  struct stmt_list* statements;
};

struct expression_stmt {
  struct stmt base;
  struct expr* expression;
};

struct if_stmt {
  struct stmt base;
  struct expr* condition;
  struct stmt* then_branch;
  struct stmt* else_branch;
};

struct print_stmt {
  struct stmt base;
  struct expr* expression;
};

struct var_stmt {
  struct stmt base;
  struct token name;
  struct expr* initializer;
};

struct block_stmt* stmt_new_block(struct stmt_list* statements);
struct expression_stmt* stmt_new_expression(struct expr* expression);
struct if_stmt* stmt_new_if(struct expr* condition,
                            struct stmt* then_branch,
                            struct stmt* else_branch);
struct print_stmt* stmt_new_print(struct expr* expression);
struct var_stmt* stmt_new_var(struct token name, struct expr* initializer);

#define STMT_DECLARE_ACCEPT_FOR(result_type, visitor_type) \
  result_type stmt_accept_##visitor_type(struct stmt* stmt, \
                                         struct visitor_type* visitor)

#define STMT_DEFINE_ACCEPT_FOR(result_type, visitor_type) \
  STMT_DECLARE_ACCEPT_FOR(result_type, visitor_type) \
  { \
    switch (stmt->type) { \
      case STMT_BLOCK: \
        return visitor_type##_visit_block_stmt(visitor, \
                                               (struct block_stmt*)stmt); \
      case STMT_EXPRESSION: \
        return visitor_type##_visit_expression_stmt( \
            visitor, (struct expression_stmt*)stmt); \
      case STMT_IF: \
        return visitor_type##_visit_if_stmt(visitor, (struct if_stmt*)stmt); \
      case STMT_PRINT: \
        return visitor_type##_visit_print_stmt(visitor, \
                                               (struct print_stmt*)stmt); \
      case STMT_VAR: \
        return visitor_type##_visit_var_stmt(visitor, (struct var_stmt*)stmt); \
    } \
    assert(false); \
  }
