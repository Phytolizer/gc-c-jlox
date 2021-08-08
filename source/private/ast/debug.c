#include <private/ast/debug.h>
#include <stdio.h>

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
      object_print(&literal->value);
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
  }
}

void stmt_debug(struct stmt* stmt)
{
  switch (stmt->type) {
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
  }
}
