#include <gc.h>
#include <private/token.h>

#define TOKEN_LIST_INITIAL_CAPACITY 8

size_t token_print(const struct token* tok)
{
  return token_fprint(stdout, tok);
}

size_t token_fprint(FILE* f, const struct token* tok)
{
  size_t len = fprintf(f, "%s %s ", TOKEN_TYPE_STRINGS[tok->type], tok->lexeme);
  return len + object_fprint(f, tok->literal);
}

size_t token_snprint(char* s, size_t n, const struct token* tok)
{
  size_t len =
      snprintf(s, n, "%s %s ", TOKEN_TYPE_STRINGS[tok->type], tok->lexeme);
  return len + object_snprint(s + len, n, tok->literal);
}

struct token_list* token_list_new(void)
{
  struct token_list* result = GC_MALLOC(sizeof(struct token_list));
  LIST_INIT(result);
  return result;
}
