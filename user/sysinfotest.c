#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int
main(void)
{
  struct sysinfo info;

  if (sysinfo(&info) < 0) {
    fprintf(2, "sysinfo: failed\n");
    exit(1);
  }

  printf("Free memory: %lu bytes\n", info.freemem);
  printf("Processes: %lu\n", info.nproc);
  printf("Open files: %lu\n", info.nopenfile);
  if (sysinfo((struct sysinfo *)0) == -1)
   printf("Invalid pointer test: PASSED\n");
  else
   printf("Invalid pointer test: FAILED\n");
  exit(0);
}
