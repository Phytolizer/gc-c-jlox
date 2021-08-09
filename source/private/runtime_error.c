#include <gc.h>
#include <private/runtime_error.h>

struct runtime_error* runtime_error_new(struct token* token,
                                        const char* message)
{
  struct runtime_error* error = GC_MALLOC(sizeof(struct runtime_error));
  error->token = token;
  error->message = message;
  return error;
}
