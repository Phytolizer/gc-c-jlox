#include <gc.h>
#include <lib.h>
#include <private/scanner.h>
#include <private/token.h>
#include <stdbool.h>
#include <string.h>

static bool scanner_is_at_end(struct scanner* self);
static void scanner_scan_token(struct scanner* self);
static char scanner_advance(struct scanner* self);
static void scanner_add_token(struct scanner* self,
                              enum token_type type,
                              struct object value);
static bool scanner_match(struct scanner* self, char expected);

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
    default:
      library_error(self->line, "Unexpected character");
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
