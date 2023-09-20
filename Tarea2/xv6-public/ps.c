#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"
#include "pstat.h"
 
int
main(void)
{
  struct pstat pinfo = {0};
  if (getpinfo(&pinfo) == -1) {
      printf(2, "Could not get process information.\n");
  }

  int i;
  for (i = 0; i < NPROC; i++) {
      if (pinfo.pid[i] == 0) continue;
      printf(1, "pid %d\n", pinfo.pid[i]);
      printf(1, "  in use: %s\n", pinfo.inuse[i] == 1 ? "Y" : "N");
      printf(1, "  ticks: %d\n", pinfo.ticks[i]);
      printf(1, "  tickets: %d\n", pinfo.tickets[i]);
  }

  exit();
}