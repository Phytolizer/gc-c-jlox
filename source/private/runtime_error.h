#pragma once

struct runtime_error {
  struct token* token;
  const char* message;
};
