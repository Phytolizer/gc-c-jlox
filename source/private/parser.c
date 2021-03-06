#include <gc.h>
#include <lib.h>
#include <private/ast/expr.h>
#include <private/parser.h>
#include <private/strutils.h>
#include <stdarg.h>
#include <stdbool.h>

#include "private/ast/stmt.h"

static struct stmt* parse_declaration(struct parser* parser);
static struct stmt* parse_statement(struct parser* parser);
static struct stmt* parse_print_statement(struct parser* parser);
static struct stmt_list* parse_block(struct parser* parser);
static struct stmt* parse_expression_statement(struct parser* parser);
static struct stmt* parse_if_statement(struct parser* parser);
static struct stmt* parse_for_statement(struct parser* parser);
static struct stmt* parse_function(struct parser* parser, const char* kind);
static struct stmt* parse_return_statement(struct parser* parser);
static struct stmt* parse_var_declaration(struct parser* parser);
static struct stmt* parse_while_statement(struct parser* parser);

static struct expr* parse_expression(struct parser* parser);
static struct expr* parse_assignment(struct parser* parser);
static struct expr* parse_or(struct parser* parser);
static struct expr* parse_and(struct parser* parser);
static struct expr* parse_equality(struct parser* parser);
static struct expr* parse_comparison(struct parser* parser);
static struct expr* parse_term(struct parser* parser);
static struct expr* parse_factor(struct parser* parser);
static struct expr* parse_unary(struct parser* parser);
static struct expr* parse_call(struct parser* parser);
static struct expr* finish_call(struct parser* parser, struct expr* callee);
static struct expr* parse_primary(struct parser* parser);

static struct token* parser_previous(struct parser* parser);
static bool parser_match(struct parser* parser, size_t n, ...);
static bool parser_check(struct parser* parser, enum token_type type);
static struct token* parser_advance(struct parser* parser);
static bool parser_is_at_end(struct parser* parser);
static struct token* parser_peek(struct parser* parser);
static struct token* parser_consume(struct parser* parser,
                                    enum token_type type,
                                    const char* message);
static void error(struct token* token, const char* message);
static void parser_synchronize(struct parser* parser);

struct parser* parser_new(struct token_list* tokens)
{
  struct parser* parser = GC_MALLOC(sizeof(struct parser));
  parser->tokens = tokens;
  parser->current = 0;
  return parser;
}

struct stmt_list* parser_parse(struct parser* parser)
{
  struct stmt_list* statements = stmt_list_new();
  while (!parser_is_at_end(parser)) {
    struct stmt* statement = parse_declaration(parser);
    if (statement) {
      LIST_PUSH(statements, statement);
    }
  }

  return statements;
}

static struct stmt* parse_declaration(struct parser* parser)
{
  if (parser_match(parser, 1, TOKEN_FUN)) {
    struct stmt* decl = parse_function(parser, "function");
    if (decl) {
      return decl;
    }
  } else if (parser_match(parser, 1, TOKEN_VAR)) {
    struct stmt* decl = parse_var_declaration(parser);
    if (decl) {
      return decl;
    }
  } else {
    struct stmt* stmt = parse_statement(parser);
    if (stmt) {
      return stmt;
    }
  }
  parser_synchronize(parser);
  return NULL;
}

static struct stmt* parse_statement(struct parser* parser)
{
  if (parser_match(parser, 1, TOKEN_IF)) {
    return parse_if_statement(parser);
  }
  if (parser_match(parser, 1, TOKEN_FOR)) {
    return parse_for_statement(parser);
  }
  if (parser_match(parser, 1, TOKEN_PRINT)) {
    return parse_print_statement(parser);
  }
  if (parser_match(parser, 1, TOKEN_RETURN)) {
    return parse_return_statement(parser);
  }
  if (parser_match(parser, 1, TOKEN_WHILE)) {
    return parse_while_statement(parser);
  }
  if (parser_match(parser, 1, TOKEN_LEFT_BRACE)) {
    struct stmt_list* statements = parse_block(parser);
    if (!statements) {
      return NULL;
    }
    return (struct stmt*)stmt_new_block(statements);
  }

  return parse_expression_statement(parser);
}

static struct stmt* parse_print_statement(struct parser* parser)
{
  struct expr* value = parse_expression(parser);
  if (!value) {
    return NULL;
  }
  if (!parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after value.")) {
    return NULL;
  }
  return (struct stmt*)stmt_new_print(value);
}

static struct stmt_list* parse_block(struct parser* parser)
{
  struct stmt_list* statements = stmt_list_new();

  while (!parser_check(parser, TOKEN_RIGHT_BRACE) && !parser_is_at_end(parser))
  {
    struct stmt* statement = parse_declaration(parser);
    if (!statement) {
      return NULL;
    }
    LIST_PUSH(statements, statement);
  }

  if (!parser_consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after block.")) {
    return NULL;
  }
  return statements;
}

static struct stmt* parse_expression_statement(struct parser* parser)
{
  struct expr* expr = parse_expression(parser);
  if (!expr) {
    return NULL;
  }
  if (!parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression."))
  {
    return NULL;
  }
  return (struct stmt*)stmt_new_expression(expr);
}

static struct stmt* parse_if_statement(struct parser* parser)
{
  if (!parser_consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'if'.")) {
    return NULL;
  }
  struct expr* condition = parse_expression(parser);
  if (!condition) {
    return NULL;
  }
  if (!parser_consume(
          parser, TOKEN_RIGHT_PAREN, "Expect ')' after if condition."))
  {
    return NULL;
  }

  struct stmt* then_branch = parse_statement(parser);
  if (!then_branch) {
    return NULL;
  }
  struct stmt* else_branch = NULL;
  if (parser_match(parser, 1, TOKEN_ELSE)) {
    else_branch = parse_statement(parser);
    if (!else_branch) {
      return NULL;
    }
  }

  return (struct stmt*)stmt_new_if(condition, then_branch, else_branch);
}

static struct stmt* parse_for_statement(struct parser* parser)
{
  if (!parser_consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'for'.")) {
    return NULL;
  }

  struct stmt* initializer;
  if (parser_match(parser, 1, TOKEN_SEMICOLON)) {
    initializer = NULL;
  } else if (parser_match(parser, 1, TOKEN_VAR)) {
    initializer = parse_var_declaration(parser);
    if (!initializer) {
      return NULL;
    }
  } else {
    initializer = parse_expression_statement(parser);
    if (!initializer) {
      return NULL;
    }
  }

  struct expr* condition = NULL;
  if (!parser_check(parser, TOKEN_SEMICOLON)) {
    condition = parse_expression(parser);
    if (!condition) {
      return NULL;
    }
  }
  if (!parser_consume(
          parser, TOKEN_SEMICOLON, "Expect ';' after loop condition."))
  {
    return NULL;
  }

  struct expr* increment = NULL;
  if (!parser_check(parser, TOKEN_RIGHT_PAREN)) {
    increment = parse_expression(parser);
    if (!increment) {
      return NULL;
    }
  }
  if (!parser_consume(
          parser, TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.")) {
    return NULL;
  }

  struct stmt* body = parse_statement(parser);
  if (!body) {
    return NULL;
  }

  if (increment) {
    struct stmt_list* new_body = stmt_list_new();
    LIST_PUSH(new_body, body);
    LIST_PUSH(new_body, (struct stmt*)stmt_new_expression(increment));
    body = (struct stmt*)stmt_new_block(new_body);
  }

  if (!condition) {
    condition = (struct expr*)expr_new_literal(OBJECT_BOOL(true));
  }
  body = (struct stmt*)stmt_new_while(condition, body);
  if (initializer) {
    struct stmt_list* new_body = stmt_list_new();
    LIST_PUSH(new_body, initializer);
    LIST_PUSH(new_body, body);
    body = (struct stmt*)stmt_new_block(new_body);
  }
  return body;
}

#define PARAMS_MAX 255

static struct stmt* parse_function(struct parser* parser, const char* kind)
{
  struct token* name = parser_consume(
      parser, TOKEN_IDENTIFIER, alloc_printf("Expect %s name.", kind));
  if (!name) {
    return NULL;
  }
  if (!parser_consume(parser,
                      TOKEN_LEFT_PAREN,
                      alloc_printf("Expect '(' after %s name.", kind)))
  {
    return NULL;
  }
  struct token_list* parameters = token_list_new();
  if (!parser_check(parser, TOKEN_RIGHT_PAREN)) {
    while (true) {
      if (parameters->length >= PARAMS_MAX) {
        error(parser_peek(parser),
              alloc_printf("Can't have more than %d parameters.", PARAMS_MAX));
      }

      struct token* param =
          parser_consume(parser, TOKEN_IDENTIFIER, "Expect parameter name.");
      if (!param) {
        return NULL;
      }
      LIST_PUSH(parameters, *param);
      if (!parser_match(parser, 1, TOKEN_COMMA)) {
        break;
      }
    }
  }
  if (!parser_consume(
          parser, TOKEN_RIGHT_PAREN, "Expect ')' after parameters.")) {
    return NULL;
  }
  if (!parser_consume(parser,
                      TOKEN_LEFT_BRACE,
                      alloc_printf("Expect '{' before %s body.", kind)))
  {
    return NULL;
  }
  struct stmt_list* body = parse_block(parser);
  return (struct stmt*)stmt_new_function(*name, parameters, body);
}

static struct stmt* parse_return_statement(struct parser* parser)
{
  struct token* keyword = parser_previous(parser);
  struct expr* value = NULL;
  if (!parser_check(parser, TOKEN_SEMICOLON)) {
    value = parse_expression(parser);
    if (!value) {
      return NULL;
    }
  }
  if (!parser_consume(
          parser, TOKEN_SEMICOLON, "Expect ';' after return value.")) {
    return NULL;
  }
  return (struct stmt*)stmt_new_return(*keyword, value);
}

static struct stmt* parse_var_declaration(struct parser* parser)
{
  struct token* name =
      parser_consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
  if (!name) {
    return NULL;
  }
  struct expr* initializer = NULL;
  if (parser_match(parser, 1, TOKEN_EQUAL)) {
    initializer = parse_expression(parser);
    if (!initializer) {
      return NULL;
    }
  }

  if (!parser_consume(
          parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration."))
  {
    return NULL;
  }
  return (struct stmt*)stmt_new_var(*name, initializer);
}

static struct stmt* parse_while_statement(struct parser* parser)
{
  if (!parser_consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.")) {
    return NULL;
  }
  struct expr* condition = parse_expression(parser);
  if (!condition) {
    return NULL;
  }
  if (!parser_consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after condition."))
  {
    return NULL;
  }
  struct stmt* body = parse_statement(parser);
  if (!body) {
    return NULL;
  }

  return (struct stmt*)stmt_new_while(condition, body);
}

static struct expr* parse_expression(struct parser* parser)
{
  return parse_assignment(parser);
}

static struct expr* parse_assignment(struct parser* parser)
{
  struct expr* expr = parse_or(parser);

  if (parser_match(parser, 1, TOKEN_EQUAL)) {
    struct token* equals = parser_previous(parser);
    struct expr* value = parse_assignment(parser);

    if (expr->type == EXPR_VARIABLE) {
      struct token name = ((struct variable_expr*)expr)->name;
      return (struct expr*)expr_new_assign(name, value);
    }

    error(equals, "Invalid assignment target.");
  }

  return expr;
}

static struct expr* parse_or(struct parser* parser)
{
  struct expr* expr = parse_and(parser);

  while (parser_match(parser, 1, TOKEN_OR)) {
    struct token* op = parser_previous(parser);
    struct expr* right = parse_and(parser);
    expr = (struct expr*)expr_new_logical(expr, *op, right);
  }

  return expr;
}

static struct expr* parse_and(struct parser* parser)
{
  struct expr* expr = parse_equality(parser);

  while (parser_match(parser, 1, TOKEN_AND)) {
    struct token* op = parser_previous(parser);
    struct expr* right = parse_equality(parser);
    expr = (struct expr*)expr_new_logical(expr, *op, right);
  }

  return expr;
}

static struct expr* parse_equality(struct parser* parser)
{
  struct expr* expr = parse_comparison(parser);
  if (!expr) {
    return NULL;
  }

  while (parser_match(parser, 2, TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL)) {
    struct token op = *parser_previous(parser);
    struct expr* right = parse_comparison(parser);
    if (!right) {
      return NULL;
    }
    expr = (struct expr*)expr_new_binary(expr, op, right);
  }

  return expr;
}

static struct expr* parse_comparison(struct parser* parser)
{
  struct expr* expr = parse_term(parser);
  if (!expr) {
    return NULL;
  }

  while (parser_match(parser,
                      4,
                      TOKEN_GREATER,
                      TOKEN_GREATER_EQUAL,
                      TOKEN_LESS,
                      TOKEN_LESS_EQUAL))
  {
    struct token op = *parser_previous(parser);
    struct expr* right = parse_term(parser);
    if (!right) {
      return NULL;
    }
    expr = (struct expr*)expr_new_binary(expr, op, right);
  }

  return expr;
}

static struct expr* parse_term(struct parser* parser)
{
  struct expr* expr = parse_factor(parser);
  if (!expr) {
    return NULL;
  }

  while (parser_match(parser, 2, TOKEN_MINUS, TOKEN_PLUS)) {
    struct token op = *parser_previous(parser);
    struct expr* right = parse_factor(parser);
    if (!right) {
      return NULL;
    }
    expr = (struct expr*)expr_new_binary(expr, op, right);
  }

  return expr;
}

static struct expr* parse_factor(struct parser* parser)
{
  struct expr* expr = parse_unary(parser);
  if (!expr) {
    return NULL;
  }

  while (parser_match(parser, 2, TOKEN_SLASH, TOKEN_STAR)) {
    struct token op = *parser_previous(parser);
    struct expr* right = parse_unary(parser);
    if (!right) {
      return NULL;
    }
    expr = (struct expr*)expr_new_binary(expr, op, right);
  }

  return expr;
}

static struct expr* parse_unary(struct parser* parser)
{
  if (parser_match(parser, 2, TOKEN_BANG, TOKEN_MINUS)) {
    struct token op = *parser_previous(parser);
    struct expr* right = parse_unary(parser);
    if (!right) {
      return NULL;
    }
    return (struct expr*)expr_new_unary(op, right);
  }

  return parse_call(parser);
}

static struct expr* parse_call(struct parser* parser)
{
  struct expr* expr = parse_primary(parser);
  if (!expr) {
    return NULL;
  }

  while (true) {
    if (parser_match(parser, 1, TOKEN_LEFT_PAREN)) {
      expr = finish_call(parser, expr);
      if (!expr) {
        return NULL;
      }
    } else {
      break;
    }
  }

  return expr;
}

#define MAX_ARGUMENTS 255

static struct expr* finish_call(struct parser* parser, struct expr* callee)
{
  struct expr_list* arguments = GC_MALLOC(sizeof(struct expr_list));
  LIST_INIT(arguments);
  if (!parser_check(parser, TOKEN_RIGHT_PAREN)) {
    while (true) {
      if (arguments->length >= MAX_ARGUMENTS) {
        error(parser_peek(parser), "Can't have more than 255 arguments.");
      }
      struct expr* expression = parse_expression(parser);
      if (!expression) {
        return NULL;
      }
      LIST_PUSH(arguments, expression);
      if (!parser_match(parser, 1, TOKEN_COMMA)) {
        break;
      }
    }
  }

  struct token* paren =
      parser_consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
  if (!paren) {
    return NULL;
  }
  return (struct expr*)expr_new_call(callee, *paren, arguments);
}

static struct expr* parse_primary(struct parser* parser)
{
  if (parser_match(parser, 1, TOKEN_FALSE)) {
    return (struct expr*)expr_new_literal(object_new_bool(false));
  }
  if (parser_match(parser, 1, TOKEN_TRUE)) {
    return (struct expr*)expr_new_literal(object_new_bool(true));
  }
  if (parser_match(parser, 1, TOKEN_NIL)) {
    return (struct expr*)expr_new_literal(object_new_null());
  }

  if (parser_match(parser, 2, TOKEN_NUMBER, TOKEN_STRING)) {
    return (struct expr*)expr_new_literal(parser_previous(parser)->literal);
  }

  if (parser_match(parser, 1, TOKEN_IDENTIFIER)) {
    return (struct expr*)expr_new_variable(*parser_previous(parser));
  }

  if (parser_match(parser, 1, TOKEN_LEFT_PAREN)) {
    struct expr* expr = parse_expression(parser);
    parser_consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
    return (struct expr*)expr_new_grouping(expr);
  }

  error(parser_peek(parser), "Expect expression.");
  return NULL;
}

static struct token* parser_previous(struct parser* parser)
{
  return &parser->tokens->pointer[parser->current - 1];
}

static bool parser_match(struct parser* parser, size_t n, ...)
{
  va_list args;
  va_start(args, n);
  for (size_t i = 0; i < n; ++i) {
    if (parser_check(parser, va_arg(args, enum token_type))) {
      parser_advance(parser);
      return true;
    }
  }

  return false;
}

static bool parser_check(struct parser* parser, enum token_type type)
{
  if (parser_is_at_end(parser)) {
    return false;
  }
  return parser_peek(parser)->type == type;
}

static struct token* parser_advance(struct parser* parser)
{
  if (!parser_is_at_end(parser)) {
    ++parser->current;
  }
  return parser_previous(parser);
}

static bool parser_is_at_end(struct parser* parser)
{
  return parser_peek(parser)->type == TOKEN_EOF;
}

static struct token* parser_peek(struct parser* parser)
{
  return &parser->tokens->pointer[parser->current];
}

static struct token* parser_consume(struct parser* parser,
                                    enum token_type type,
                                    const char* message)
{
  if (parser_check(parser, type)) {
    return parser_advance(parser);
  }

  error(parser_peek(parser), message);
  return NULL;
}

static void error(struct token* token, const char* message)
{
  library_error_at_token(token, message);
}

static void parser_synchronize(struct parser* parser)
{
  parser_advance(parser);
  while (!parser_is_at_end(parser)) {
    if (parser_previous(parser)->type == TOKEN_SEMICOLON) {
      return;
    }

    switch (parser_peek(parser)->type) {
      case TOKEN_CLASS:
      case TOKEN_FOR:
      case TOKEN_FUN:
      case TOKEN_IF:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
      case TOKEN_VAR:
      case TOKEN_WHILE:
        return;
      default:
        // doesn't break loop
        break;
    }
    parser_advance(parser);
  }
}
