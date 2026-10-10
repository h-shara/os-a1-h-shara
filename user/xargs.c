
#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define BUFSIZE 4096

char input[BUFSIZE];
char *args[MAXARG + 1];

void usage(void)
{
  fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
  exit(1);
}

int run(char *cmd, int base, int count)
{
  args[base + count] = 0;

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "xargs: fork failed\n");
    return 1;
  }

  if (pid == 0) {
    exec(cmd, args);
    fprintf(2, "xargs: exec failed\n");
    exit(1);
  }

  int status;
  if (wait(&status) < 0)
    return 1;

  return status != 0;
}

int main(int argc, char *argv[])
{
  int i = 1;
  int limit = MAXARG;
  int use_n = 0;
  int failed = 0;

  if (argc > 1 && strcmp(argv[1], "-n") == 0) {
    use_n = 1;

    if (argc < 4)
      usage();

    if (argv[2][0] == '\0')
      usage();

    for (int j = 0; argv[2][j]; j++) {
      if (argv[2][j] < '0' || argv[2][j] > '9')
        usage();
    }

    limit = atoi(argv[2]);
    if (limit < 1)
      usage();

    i = 3;
  }

  if (i >= argc)
    usage();

  int base = argc - i;
  if (base > MAXARG)
    usage();

  for (int j = 0; j < base; j++)
    args[j] = argv[i + j];

  int maxwords = MAXARG - base;
  if (maxwords < 1)
    usage();

  if (use_n && limit < maxwords)
    maxwords = limit;

  int pos = 0;
  int count = 0;
  int inword = 0;
  int c;
  int r;

  while ((r = read(0, &c, 1)) == 1) {
    char ch = (char)c;
    int separator = (ch == ' ' || ch == '\t' ||
                     ch == '\n' || ch == '\r');

    if (separator) {
      if (inword) {
        if (pos >= BUFSIZE)
          usage();
        input[pos++] = '\0';
        inword = 0;
        count++;
      }

      if (count == maxwords ||
          (!use_n && ch == '\n' && count > 0)) {
        int start = 0;
        int n = 0;

        while (start < pos) {
          args[base + n++] = &input[start];
          start += strlen(&input[start]) + 1;
        }

        if (run(args[0], base, n))
          failed = 1;

        pos = 0;
        count = 0;
      }
    } else {
      if (pos >= BUFSIZE - 1)
        usage();
      input[pos++] = ch;
      inword = 1;
    }
  }

  if (r < 0) {
    fprintf(2, "xargs: read failed\n");
    exit(1);
  }

  if (inword) {
    input[pos++] = '\0';
    count++;
  }

  if (count > 0) {
    int start = 0;
    int n = 0;

    while (start < pos) {
      args[base + n++] = &input[start];
      start += strlen(&input[start]) + 1;
    }

    if (run(args[0], base, n))
      failed = 1;
  }

  exit(failed ? 1 : 0);
}
