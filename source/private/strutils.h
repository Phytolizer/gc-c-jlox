#pragma once

char* alloc_printf(const char* format, ...)
    __attribute__((format(printf, 1, 2)));
