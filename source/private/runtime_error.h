#pragma once

struct runtime_error {
  struct token* token;
  const char* message;
};

struct runtime_error* runtime_error_new(struct token* token,
                                        const char* message);
