#include <lib.h>
#include <string.h>

int main(int argc, const char* argv[])
{
  (void)argc;
  (void)argv;

  library lib = create_library();

  return strcmp(lib.name, "gc-c-jlox") == 0 ? 0 : 1;
}
