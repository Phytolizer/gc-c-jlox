#include <private/ast/debug.h>
#include <stdio.h>

#include "private/token.h"

size_t AstDebugIndentLevel = 0;

void ast_debug_print_indent(void)
{
  for (size_t i = 0; i < AstDebugIndentLevel; ++i) {
    printf("  ");
  }
}

void expr_debug(struct expr* expr)
{
  switch (expr->type) {
    case EXPR_ASSIGN: {
      struct assign_expr* assign = (struct assign_expr*)expr;
      printf("ASSIGN_EXPR {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".name = ");
      token_print(&assign->name);
      printf(",\n");
      ast_debug_print_indent();
      printf(".value = ");
      expr_debug(assign->value);
      printf(",\n");
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
    case EXPR_BINARY: {
      struct binary_expr* binary = (struct binary_expr*)expr;
      printf("BINARY_EXPR {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".left = ");
      expr_debug(binary->left);
      printf(",\n");
      ast_debug_print_indent();
      printf(".operator = ");
      token_print(&binary->op);
      printf(",\n");
      ast_debug_print_indent();
      printf(".right = ");
      expr_debug(binary->right);
      printf(",\n");
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
    case EXPR_CALL: {
      struct call_expr* call = (struct call_expr*)expr;
      printf("CALL_EXPR {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".callee = ");
      expr_debug(call->callee);
      printf(",\n");
      ast_debug_print_indent();
      printf(".arguments = [\n");
      ++AstDebugIndentLevel;
      for (long i = 0; i < call->arguments->length; ++i) {
        ast_debug_print_indent();
        expr_debug(call->arguments->pointer[i]);
        printf(",\n");
      }
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("],\n");
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
    case EXPR_GROUPING: {
      struct grouping_expr* grouping = (struct grouping_expr*)expr;
      printf("GROUPING_EXPR {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".expression = ");
      expr_debug(grouping->expression);
      printf(",\n");
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
    case EXPR_LITERAL: {
      struct literal_expr* literal = (struct literal_expr*)expr;
      printf("LITERAL_EXPR {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".value = ");
      object_print(literal->value);
      printf(",\n");
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
    case EXPR_LOGICAL: {
      struct logical_expr* logical = (struct logical_expr*)expr;
      printf("LOGICAL_EXPR {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".left = ");
      expr_debug(logical->left);
      printf(",\n");
      ast_debug_print_indent();
      printf(".op = ");
      token_print(&logical->op);
      printf(",\n");
      ast_debug_print_indent();
      printf(".right = ");
      expr_debug(logical->right);
      printf(",\n");
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
    case EXPR_UNARY: {
      struct unary_expr* unary = (struct unary_expr*)expr;
      printf("UNARY_EXPR {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".op = ");
      token_print(&unary->op);
      printf(",\n");
      ast_debug_print_indent();
      printf(".right = ");
      expr_debug(unary->right);
      printf(",\n");
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
    case EXPR_VARIABLE: {
      struct variable_expr* variable = (struct variable_expr*)expr;
      printf("VARIABLE_EXPR {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".name = ");
      token_print(&variable->name);
      printf(",\n");
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
  }
}

void stmt_debug(struct stmt* stmt)
{
  switch (stmt->type) {
    case STMT_BLOCK: {
      struct block_stmt* block = (struct block_stmt*)stmt;
      printf("BLOCK_STMT {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".statements = {\n");
      ++AstDebugIndentLevel;
      for (long i = 0; i < block->statements->length; ++i) {
        ast_debug_print_indent();
        stmt_debug(block->statements->pointer[i]);
        printf(",\n");
      }
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("},\n");
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
    case STMT_EXPRESSION: {
      struct expression_stmt* expression = (struct expression_stmt*)stmt;
      printf("EXPRESSION_STMT {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".expression = ");
      expr_debug(expression->expression);
      printf(",\n");
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
    case STMT_IF: {
      struct if_stmt* ifs = (struct if_stmt*)stmt;
      printf("IF_STMT {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".condition = ");
      expr_debug(ifs->condition);
      printf(",\n");
      ast_debug_print_indent();
      printf(".then_branch = ");
      stmt_debug(ifs->then_branch);
      printf(",\n");
      ast_debug_print_indent();
      printf(".else_branch = ");
      if (ifs->else_branch) {
        stmt_debug(ifs->else_branch);
      } else {
        printf("NULL");
      }
      printf(",\n");
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
    case STMT_PRINT: {
      struct print_stmt* print = (struct print_stmt*)stmt;
      printf("PRINT_STMT {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".expression = ");
      expr_debug(print->expression);
      printf(",\n");
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
    case STMT_VAR: {
      struct var_stmt* var = (struct var_stmt*)stmt;
      printf("VAR_STMT {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".name = ");
      token_print(&var->name);
      printf(",\n");
      if (var->initializer) {
        ast_debug_print_indent();
        printf(".initializer = ");
        expr_debug(var->initializer);
        printf(",\n");
      }
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
    case STMT_WHILE: {
      struct while_stmt* ws = (struct while_stmt*)stmt;
      printf("WHILE_STMT {\n");
      ++AstDebugIndentLevel;
      ast_debug_print_indent();
      printf(".condition = ");
      expr_debug(ws->condition);
      printf(",\n");
      ast_debug_print_indent();
      printf(".body = ");
      stmt_debug(ws->body);
      printf(",\n");
      --AstDebugIndentLevel;
      ast_debug_print_indent();
      printf("}");
      break;
    }
  }
}
