#include <assert.h>
#include <errno.h>
#include <gc.h>
#include <lib.h>
#include <private/ast/debug.h>
#include <private/ast/expr.h>
#include <private/ast/printer.h>
#include <private/interpreter.h>
#include <private/parser.h>
#include <private/scanner.h>
#include <private/strutils.h>
#include <private/token.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

bool HadError = false;
bool HadRuntimeError = false;
static struct interpreter* interpreter = NULL;

static void report(size_t line, const char* where, const char* message);

static void library_run(const char* text_begin, const char* text_end)
{
  if (!interpreter) {
    interpreter = interpreter_new();
  }
  struct scanner* scanner = scanner_new(text_begin, text_end);
  struct token_list* tokens = scanner_scan_tokens(scanner);
  struct parser* parser = parser_new(tokens);
  struct stmt_list* statements = parser_parse(parser);
  if (HadError) {
    return;
  }
  printf("--- STATEMENTS ---\n");
  for (long i = 0; i < statements->length; i++) {
    stmt_debug(statements->pointer[i]);
    printf("\n");
  }
  printf("--- END STATEMENTS ---\n");
  interpret(interpreter, statements);
}

int library_run_file(const char* filename)
{
  FILE* fp = fopen(filename, "re");
  if (!fp) {
    return EX_NOINPUT;
  }

  int seekret = fseek(fp, 0, SEEK_END);
  long eofpos = ftell(fp);
  if (seekret == -1 || eofpos == -1) {
    // this will ALWAYS crash, as the args are valid
    assert(errno != EINVAL);
    assert(errno != ESPIPE);
    fclose(fp);
    return EX_OSERR;
  }

  rewind(fp);
  char* contents = GC_MALLOC((size_t)eofpos + 1);
  if (!contents) {
    fclose(fp);
    fprintf(stderr, "Could not allocate enough memory for file contents\n");
    fprintf(stderr, "  (file name: %s)\n", filename);
    return EX_OSERR;
  }

  size_t nread = fread(contents, 1, eofpos, fp);
  if (nread != (size_t)eofpos) {
    if (feof(fp)) {
      fprintf(stderr,
              "EOF encountered early when reading file. "
              "Probably the file was "
              "modified during execution.\n");
    } else {
      fprintf(stderr,
              "Error occurred while reading file (code "
              "%zu)\n",
              nread);
    }
    fprintf(stderr, "  (file name: %s)\n", filename);
    fclose(fp);
    return EX_OSERR;
  }

  fclose(fp);
  contents[eofpos] = '\0';
  library_run(contents, contents + eofpos);

  if (HadError) {
    return EX_DATAERR;
  }
  if (HadRuntimeError) {
    return EX_SOFTWARE;
  }
  return 0;
}

int library_run_prompt()
{
  while (true) {
    printf("> ");
    fflush(stdout);
    char* line = NULL;
    size_t len;
    ssize_t ret = getline(&line, &len, stdin);
    if (ret == -1) {
      assert(errno != EINVAL);
      if (errno == 0) {
        printf("\n");
        free(line);
        break;
      }
      fprintf(stderr, "Couldn't allocate enough memory for line input.\n");
      free(line);
      return EX_OSERR;
    }

    library_run(line, line + strlen(line));

    HadError = false;
    HadRuntimeError = false;

    free(line);
  }
  return 0;
}

void library_error(size_t line, const char* message)
{
  report(line, "", message);
}

void library_error_at_token(struct token* token, const char* message)
{
  if (token->type == TOKEN_EOF) {
    report(token->line, " at end", message);
  } else {
    char* where = alloc_printf(" at '%s'", token->lexeme);
    report(token->line, where, message);
  }
}

void library_runtime_error(struct runtime_error* err)
{
  fprintf(stderr, "%s\n[line %zu]\n", err->message, err->token->line);
  HadRuntimeError = true;
}

static void report(size_t line, const char* where, const char* message)
{
  fprintf(stderr, "[line %zu] Error%s: %s\n", line, where, message);
  HadError = true;
}
