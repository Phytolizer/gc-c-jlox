#include <lib.h>
#include <private/ast/expr.h>
#include <private/ast/printer.h>
#include <string.h>

static int test_ast_printer(void);

int main(int argc, const char* argv[])
{
  (void)argc;
  (void)argv;

  int ret;

  if ((ret = test_ast_printer())) {
    return ret;
  }
  return 0;
}

static int test_ast_printer(void)
{
  struct ast_printer printer;
  struct expr* expr = (struct expr*)expr_new_binary(
      (struct expr*)expr_new_literal(OBJECT_NUMBER(1)),
      (struct token) {
          .type = TOKEN_PLUS,
          .lexeme = "+",
          .literal = OBJECT_NULL(),
          .line = 1,
      },
      (struct expr*)expr_new_binary(
          (struct expr*)expr_new_literal(OBJECT_NUMBER(2)),
          (struct token) {
              .type = TOKEN_STAR,
              .lexeme = "*",
              .literal = OBJECT_NULL(),
              .line = 1,
          },
          (struct expr*)expr_new_literal(OBJECT_NUMBER(3))));
  const char* printed = expr_accept_ast_printer(expr, &printer);
  int ret = strcmp(printed, "(+ 1 (* 2 3))");
  if (ret) {
    printf("%s\n", printed);
  }
  return ret;
}
