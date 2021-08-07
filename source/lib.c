#include <assert.h>
#include <gc.h>
#include <lib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

bool HadError;

static void report(int line, const char* where, const char* message);

static int library_run(const char* text_begin, const char* text_end)
{
  Scanner* scanner = new_scanner(text_begin, text_end);
  TokenList* tokens = scanner_scan_tokens(scanner);

  for (int i = 0; i < tokens->length; ++i) {
    token_print(tokens->data[i]);
    printf("\n");
  }
}

int library_run_file(const char* filename)
{
  FILE* fp = fopen(filename, "re");
  if (!fp) {
    return EX_NOINPUT;
  }

  int eofpos = fseek(fp, 0, SEEK_END);
  if (eofpos == -1) {
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
              "EOF encountered early when reading file. Probably the file was "
              "modified during execution.\n");
    } else {
      fprintf(stderr, "Error occurred while reading file (code %zu)\n", nread);
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
  return 0;
}

void library_error(int line, const char* message)
{
  report(line, "", message);
}

static void report(int line, const char* where, const char* message)
{
  fprintf(stderr, "[line %d] Error%s: %s", line, where, message);
  HadError = true;
}
