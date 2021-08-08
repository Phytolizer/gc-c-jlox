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
  return len + object_fprint(f, &tok->literal);
}

size_t token_snprint(char* s, size_t n, const struct token* tok)
{
  size_t len =
      snprintf(s, n, "%s %s ", TOKEN_TYPE_STRINGS[tok->type], tok->lexeme);
  return len + object_snprint(s + len, n, &tok->literal);
}

struct token_list* token_list_new(void)
{
  struct token_list* result = GC_MALLOC(sizeof(struct token_list));
  result->data = NULL;
  result->length = 0;
  result->capacity = 0;
  return result;
}

void token_list_push(struct token_list* list, struct token tok)
{
  if (list->length == list->capacity) {
    if (list->capacity == 0) {
      list->capacity = TOKEN_LIST_INITIAL_CAPACITY;
    } else {
      list->capacity *= 2;
    }
    list->data = GC_REALLOC(list->data, list->capacity * sizeof(struct token));
  }

  list->data[list->length] = tok;
  ++list->length;
}
