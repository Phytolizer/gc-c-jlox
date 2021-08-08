#pragma once

#include <private/runtime_error.h>
#include <private/token.h>
#include <stdbool.h>
#include <stddef.h>

int library_run_file(const char* filename);
int library_run_prompt(void);
void library_error(size_t line, const char* message);
void library_error_at_token(struct token* token, const char* message);
void library_runtime_error(struct runtime_error* err);

extern bool HadError;
extern bool HadRuntimeError;
