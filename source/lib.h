#pragma once

#include <stdbool.h>

int library_run_file(const char* filename);
int library_run_prompt(void);
void library_error(int line, const char* message);

extern bool HadError;
