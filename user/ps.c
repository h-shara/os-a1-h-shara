#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/uproc.h"
#include "user/user.h"

static struct uproc procs[NPROC];

static char *
state_name(int state)
{
  switch (state) {
    case 0: return "UNUSED";
    case 1: return "USED";
    case 2: return "SLEEPING";
    case 3: return "RUNNABLE";
    case 4: return "RUNNING";
    case 5: return "ZOMBIE";
    default: return "UNKNOWN";
  }
}

int
main(int argc, char *argv[])
{
  int n;

  if (argc != 1) {
    fprintf(2, "usage: ps\n");
    exit(1);
  }

  n = getprocs(procs, NPROC);

  if (n < 0) {
    fprintf(2, "ps: getprocs failed\n");
    exit(1);
  }

  printf("PID\tPPID\tSTATE\tSIZE\tNAME\n");

  for (int i = 0; i < n; i++) {
    printf("%d\t%d\t%s\t%lu\t%s\n",
           procs[i].pid,
           procs[i].ppid,
           state_name(procs[i].state),
           procs[i].sz,
           procs[i].name);
  }

  exit(0);
}
