
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

int match(char *pattern, char *name)
{
  if (*pattern == '\0')
    return *name == '\0';

  if (*pattern == '*') {
    // '*' can match zero characters.
    if (match(pattern + 1, name))
      return 1;

    // Or it can match one or more characters.
    if (*name != '\0')
      return match(pattern, name + 1);

    return 0;
  }

  if (*name != '\0' && *pattern == *name)
    return match(pattern + 1, name + 1);

  return 0;
}

void search(char *path, char *pattern, int type)
{
  int fd;
  struct stat st;
  struct dirent de;
  char buf[512];
  char *p;

  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    exit(1);
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    close(fd);
    exit(1);
  }

  if (st.type != T_DIR) {
    close(fd);
    return;
  }

  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
    fprintf(2, "find: path too long\n");
    close(fd);
    exit(1);
  }

  strcpy(buf, path);
  p = buf + strlen(buf);
  if (p > buf && p[-1] != '/')
    *p++ = '/';

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;

    char name[DIRSIZ + 1];
    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = '\0';

    if (strcmp(name, ".") == 0 ||
        strcmp(name, "..") == 0)
      continue;

    strcpy(p, name);

    if (stat(buf, &st) < 0)
      continue;

    if (match(pattern, name)) {
      if (type == 0 ||
          (type == 1 && (st.type == T_FILE || st.type == T_DEVICE)) ||
          (type == 2 && st.type == T_DIR)) {
        printf("%s\n", buf);
      }
    }

    if (st.type == T_DIR)
      search(buf, pattern, type);
  }

  close(fd);
}

int main(int argc, char *argv[])
{
  int type = 0;

  if (argc != 3 && argc != 5) {
    fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
    exit(1);
  }

  if (argc == 5) {
    if (strcmp(argv[3], "-type") != 0) {
      fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
      exit(1);
    }

    if (strcmp(argv[4], "f") == 0)
      type = 1;
    else if (strcmp(argv[4], "d") == 0)
      type = 2;
    else {
      fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
      exit(1);
    }
  }

  search(argv[1], argv[2], type);
  exit(0);
}
