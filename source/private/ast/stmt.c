#include <gc.h>
#include <private/ast/stmt.h>

struct expression_stmt* stmt_new_expression(struct expr* expression)
{
  struct expression_stmt* stmt = GC_MALLOC(sizeof(struct expression_stmt));
  stmt->base.type = STMT_EXPRESSION;
  stmt->expression = expression;
  return stmt;
}

struct print_stmt* stmt_new_print(struct expr* expression)
{
  struct print_stmt* stmt = GC_MALLOC(sizeof(struct print_stmt));
  stmt->base.type = STMT_PRINT;
  stmt->expression = expression;
  return stmt;
}

struct var_stmt* stmt_new_var(struct token name, struct expr* initializer)
{
  struct var_stmt* stmt = GC_MALLOC(sizeof(struct var_stmt));
  stmt->base.type = STMT_VAR;
  stmt->name = name;
  stmt->initializer = initializer;
  return stmt;
}
