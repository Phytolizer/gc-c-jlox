#pragma once

#include <private/token.h>

struct scanner {
  const char* source_begin;
  const char* source_end;
  size_t start;
  size_t current;
  size_t line;
  struct token_list* tokens;
};

struct scanner* scanner_new(const char* source_begin, const char* source_end);
struct token_list* scanner_scan_tokens(struct scanner* self);
