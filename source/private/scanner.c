#include <ctype.h>
#include <gc.h>
#include <lib.h>
#include <private/scanner.h>
#include <private/token.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "private/hash/fnv.h"
#include "private/hash/table.h"

static bool scanner_is_at_end(struct scanner* self);
static void scanner_scan_token(struct scanner* self);
static char scanner_advance(struct scanner* self);
static void scanner_add_token(struct scanner* self,
                              enum token_type type,
                              struct object value);
static bool scanner_match(struct scanner* self, char expected);
static char scanner_peek(struct scanner* self);
static char scanner_peek_next(struct scanner* self);
static void scanner_string(struct scanner* self);
static void scanner_number(struct scanner* self);
static bool is_alpha(char c);
static bool is_alphanumeric(char c);
static void scanner_identifier(struct scanner* self);
static void init_keywords(void);

static struct hash_table* Keywords = NULL;

struct scanner* scanner_new(const char* source_begin, const char* source_end)
{
  struct scanner* scanner = GC_MALLOC(sizeof(struct scanner));
  scanner->source_begin = source_begin;
  scanner->source_end = source_end;
  scanner->tokens = token_list_new();
  scanner->start = 0;
  scanner->current = 0;
  scanner->line = 1;
  return scanner;
}

struct token_list* scanner_scan_tokens(struct scanner* self)
{
  while (!scanner_is_at_end(self)) {
    self->start = self->current;
    scanner_scan_token(self);
  }

  token_list_push(self->tokens,
                  (struct token) {
                      .type = TOKEN_EOF,
                      .lexeme = "",
                      .literal = OBJECT_NULL(),
                      .line = self->line,
                  });
  return self->tokens;
}

static bool scanner_is_at_end(struct scanner* self)
{
  return self->current >= self->source_end - self->source_begin;
}

static void scanner_scan_token(struct scanner* self)
{
  char c = scanner_advance(self);
  switch (c) {
    case '(':
      scanner_add_token(self, TOKEN_LEFT_PAREN, OBJECT_NULL());
      break;
    case ')':
      scanner_add_token(self, TOKEN_RIGHT_PAREN, OBJECT_NULL());
      break;
    case '{':
      scanner_add_token(self, TOKEN_LEFT_BRACE, OBJECT_NULL());
      break;
    case '}':
      scanner_add_token(self, TOKEN_RIGHT_BRACE, OBJECT_NULL());
      break;
    case ',':
      scanner_add_token(self, TOKEN_COMMA, OBJECT_NULL());
      break;
    case '.':
      scanner_add_token(self, TOKEN_DOT, OBJECT_NULL());
      break;
    case '-':
      scanner_add_token(self, TOKEN_MINUS, OBJECT_NULL());
      break;
    case '+':
      scanner_add_token(self, TOKEN_PLUS, OBJECT_NULL());
      break;
    case ';':
      scanner_add_token(self, TOKEN_SEMICOLON, OBJECT_NULL());
      break;
    case '*':
      scanner_add_token(self, TOKEN_STAR, OBJECT_NULL());
      break;
    case '!':
      scanner_add_token(
          self,
          scanner_match(self, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG,
          OBJECT_NULL());
      break;
    case '=':
      scanner_add_token(
          self,
          scanner_match(self, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL,
          OBJECT_NULL());
      break;
    case '<':
      scanner_add_token(
          self,
          scanner_match(self, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS,
          OBJECT_NULL());
      break;
    case '>':
      scanner_add_token(
          self,
          scanner_match(self, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER,
          OBJECT_NULL());
      break;
    case ' ':
    case '\r':
    case '\t':
      break;
    case '\n':
      ++self->line;
      break;
    case '/':
      if (scanner_match(self, '/')) {
        while (scanner_peek(self) != '\n' && !scanner_is_at_end(self)) {
          scanner_advance(self);
        }
      } else {
        scanner_add_token(self, TOKEN_SLASH, OBJECT_NULL());
      }
      break;
    case '"':
      scanner_string(self);
      break;
    default:
      if (isdigit(c)) {
        scanner_number(self);
      } else if (is_alpha(c)) {
        scanner_identifier(self);
      } else {
        library_error(self->line, "Unexpected character.");
      }
      break;
  }
}

static char scanner_advance(struct scanner* self)
{
  return self->source_begin[self->current++];
}

static void scanner_add_token(struct scanner* self,
                              enum token_type type,
                              struct object value)
{
  char* text = GC_MALLOC(self->current - self->start + 1);
  strncpy(text, self->source_begin + self->start, self->current - self->start);
  token_list_push(self->tokens,
                  (struct token) {
                      .type = type,
                      .lexeme = text,
                      .literal = value,
                      .line = self->line,
                  });
}

static bool scanner_match(struct scanner* self, char expected)
{
  if (scanner_is_at_end(self) || self->source_begin[self->current] != expected)
  {
    return false;
  }

  ++self->current;
  return true;
}

static char scanner_peek(struct scanner* self)
{
  if (scanner_is_at_end(self)) {
    return '\0';
  }

  return self->source_begin[self->current];
}

static char scanner_peek_next(struct scanner* self)
{
  if (self->current + 1 >= self->source_end - self->source_begin) {
    return '\0';
  }
  return self->source_begin[self->current + 1];
}

static void scanner_string(struct scanner* self)
{
  while (scanner_peek(self) != '"' && !scanner_is_at_end(self)) {
    if (scanner_peek(self) == '\n') {
      ++self->line;
    }
    scanner_advance(self);
  }

  if (scanner_is_at_end(self)) {
    library_error(self->line, "Unterminated string.");
    return;
  }

  scanner_advance(self);

  char* value = GC_MALLOC(self->current - self->start - 1);
  strncpy(value,
          self->source_begin + self->start + 1,
          self->current - self->start - 2);
  scanner_add_token(self, TOKEN_STRING, OBJECT_STRING(value));
}

static void scanner_number(struct scanner* self)
{
  while (isdigit(scanner_peek(self))) {
    scanner_advance(self);
  }

  if (scanner_peek(self) == '.' && isdigit(scanner_peek_next(self))) {
    scanner_advance(self);

    while (isdigit(scanner_peek(self))) {
      scanner_advance(self);
    }
  }

  char* value = GC_MALLOC(self->current - self->start + 1);
  strncpy(value, self->source_begin + self->start, self->current - self->start);
  double dval = strtod(value, NULL);
  scanner_add_token(self, TOKEN_NUMBER, OBJECT_NUMBER(dval));
}

static bool is_alpha(char c)
{
  return c == '_' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

static bool is_alphanumeric(char c)
{
  return isdigit(c) || is_alpha(c);
}

static void scanner_identifier(struct scanner* self)
{
  while (is_alphanumeric(scanner_peek(self))) {
    scanner_advance(self);
  }

  if (Keywords == NULL) {
    init_keywords();
  }

  enum token_type type = TOKEN_IDENTIFIER;
  int* iptr = hash_table_try_get(
      Keywords, self->source_begin + self->start, self->current - self->start);
  if (iptr) {
    type = *iptr;
  }
  scanner_add_token(self, type, OBJECT_NULL());
}

static void init_keywords(void)
{
  Keywords = hash_table_new(hash_fnv1a);
  hash_table_insert(Keywords, "and", TOKEN_AND);
  hash_table_insert(Keywords, "class", TOKEN_CLASS);
  hash_table_insert(Keywords, "else", TOKEN_ELSE);
  hash_table_insert(Keywords, "false", TOKEN_FALSE);
  hash_table_insert(Keywords, "for", TOKEN_FOR);
  hash_table_insert(Keywords, "fun", TOKEN_FUN);
  hash_table_insert(Keywords, "if", TOKEN_IF);
  hash_table_insert(Keywords, "nil", TOKEN_NIL);
  hash_table_insert(Keywords, "or", TOKEN_OR);
  hash_table_insert(Keywords, "print", TOKEN_PRINT);
  hash_table_insert(Keywords, "return", TOKEN_RETURN);
  hash_table_insert(Keywords, "super", TOKEN_SUPER);
  hash_table_insert(Keywords, "this", TOKEN_THIS);
  hash_table_insert(Keywords, "true", TOKEN_TRUE);
  hash_table_insert(Keywords, "var", TOKEN_VAR);
  hash_table_insert(Keywords, "while", TOKEN_WHILE);
}
