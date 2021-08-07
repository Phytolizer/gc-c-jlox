#include <lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <gc.h>

int main(int argc, const char* argv[])
{
  GC_INIT();
  if (argc > 2) {
    fprintf(stderr, "Usage: %s [script]\n", argv[0]);
    return EX_USAGE;
  }
  if (argc == 2) {
    return library_run_file(argv[1]);
  }
  return library_run_prompt();
}
