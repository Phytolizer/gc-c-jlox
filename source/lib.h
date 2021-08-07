#pragma once

#include <stdbool.h>
#include <stddef.h>

int library_run_file(const char* filename);
int library_run_prompt(void);
void library_error(size_t line, const char* message);

extern bool HadError;
