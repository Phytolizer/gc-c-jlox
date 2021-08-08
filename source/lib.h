#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <private/token.h>

int library_run_file(const char* filename);
int library_run_prompt(void);
void library_error(size_t line, const char* message);
void library_error_at_token(struct token* token, const char* message);

extern bool HadError;
