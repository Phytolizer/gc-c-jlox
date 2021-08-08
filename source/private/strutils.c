#include <gc.h>
#include <private/strutils.h>
#include <stdarg.h>
#include <stdio.h>

char* alloc_printf(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  size_t len = vsnprintf(NULL, 0, format, args);
  va_end(args);

  char* str = GC_MALLOC(len + 1);

  va_start(args, format);
  vsprintf(str, format, args);
  va_end(args);
  return str;
}
