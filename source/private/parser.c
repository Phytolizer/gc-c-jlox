#include <gc.h>
#include <lib.h>
#include <private/ast/expr.h>
#include <private/parser.h>
#include <stdarg.h>
#include <stdbool.h>

static struct expr* parse_expression(struct parser* parser);
static struct expr* parse_equality(struct parser* parser);
static struct expr* parse_comparison(struct parser* parser);
static struct expr* parse_term(struct parser* parser);
static struct expr* parse_factor(struct parser* parser);
static struct expr* parse_unary(struct parser* parser);
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

struct expr* parser_parse(struct parser* parser)
{
  return parse_expression(parser);
}

static struct expr* parse_expression(struct parser* parser)
{
  return parse_equality(parser);
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

  return parse_primary(parser);
}

static struct expr* parse_primary(struct parser* parser)
{
  if (parser_match(parser, 1, TOKEN_FALSE)) {
    return (struct expr*)expr_new_literal(OBJECT_BOOL(false));
  }
  if (parser_match(parser, 1, TOKEN_TRUE)) {
    return (struct expr*)expr_new_literal(OBJECT_BOOL(true));
  }
  if (parser_match(parser, 1, TOKEN_NIL)) {
    return (struct expr*)expr_new_literal(OBJECT_NULL());
  }

  if (parser_match(parser, 2, TOKEN_NUMBER, TOKEN_STRING)) {
    return (struct expr*)expr_new_literal(parser_previous(parser)->literal);
  }

  if (parser_match(parser, 1, TOKEN_LEFT_PAREN)) {
    struct expr* expr = parse_expression(parser);
    parser_consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after expresion.");
    return (struct expr*)expr_new_grouping(expr);
  }

  error(parser_peek(parser), "Expect expression.");
  return NULL;
}

static struct token* parser_previous(struct parser* parser)
{
  return &parser->tokens->data[parser->current - 1];
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
  return &parser->tokens->data[parser->current];
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
