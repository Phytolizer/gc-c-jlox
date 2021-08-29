#include <gc.h>
#include <private/ast/stmt.h>

struct stmt_list* stmt_list_new(void)
{
  struct stmt_list* list = GC_MALLOC(sizeof(struct stmt_list));
  list->pointer = NULL;
  list->length = 0;
  list->capacity = 0;
  return list;
}

struct block_stmt* stmt_new_block(struct stmt_list* statements)
{
  struct block_stmt* stmt = GC_MALLOC(sizeof(struct block_stmt));
  stmt->base.type = STMT_BLOCK;
  stmt->statements = statements;
  return stmt;
}

struct expression_stmt* stmt_new_expression(struct expr* expression)
{
  struct expression_stmt* stmt = GC_MALLOC(sizeof(struct expression_stmt));
  stmt->base.type = STMT_EXPRESSION;
  stmt->expression = expression;
  return stmt;
}

struct if_stmt* stmt_new_if(struct expr* condition,
                            struct stmt* then_branch,
                            struct stmt* else_branch)
{
  struct if_stmt* stmt = GC_MALLOC(sizeof(struct if_stmt)); 
  stmt->base.type = STMT_IF;
  stmt->condition = condition;
  stmt->then_branch = then_branch;
  stmt->else_branch = else_branch;
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
