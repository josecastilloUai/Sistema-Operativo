// Run trials to determine how much CPU share a number of processes use.
// Output is printed in a format that can be copied to Excel and used to
// create a graph. Paste Special -> Text will paste the columns correctly.

#include "types.h"
#include "stat.h"
#include "user.h"
#include "utils.h"

#define NUM_CHILDREN      3       // How many children to fork and measure
#define TICKET_MULTIPLIER 100     // Multiply ticket proportion
int TRIALS = 10;                  // How many times to run the test
int DURATION = 100;               // Duration (in ticks) to run each child

struct test_child {
  int pid;
  int tick_share;
  int tickets;
};

int total_tickets;
struct test_child tc[NUM_CHILDREN];

// Get the number of ticks a given process has run for
int 
get_ticks(int pid) 
{
  struct pstat pinfo = {0};
  if (getpinfo(&pinfo) == -1) {
    printf(2, "Could not get process information.\n");
  }

  int k;
  for (k = 0; k < NPROC; k++) {
    if (pinfo.pid[k] == getpid())
      return pinfo.ticks[k];
  }

  return -1;
}

// Run a trial which spawns NUM_CHILDREN processes and measures their tick share
void
run_trial(int trial_num)
{
  int i;
  int start_ticks = uptime();

  // Assign tickets to each child
  for (i = 0; i < NUM_CHILDREN; i++) {
    int assigned_tickets = (i+1)*TICKET_MULTIPLIER;
    tc[i].tickets = assigned_tickets;
    tc[i].tick_share = -1;
  }
  
  printf(1, "run %d:", trial_num);
  for (i = 0; i < NUM_CHILDREN; i++) {
    // Set the tickets in the parent before forking the child (child inherits tickets)
    if (settickets(tc[i].tickets) == -1) {
        printf(2, "Could not set ticket count.\n");
        exit();
    }

    if ((tc[i].pid = fork()) == 0) {
        while (tc[i].tick_share == -1) {
            // Once child has been reached the duration, save tick share
            if (uptime() - start_ticks >= DURATION) {
              int ticks = get_ticks(getpid());
              if (ticks == -1) {
                printf(2, "Could not get child tick count for child #%d (pid %d).\n", i, getpid());
                exit();
              }

              tc[i].tick_share = ticks;
            }
        }

        sleep(tc[i].tickets/20);

        // Print this child's tick share
        int j;
        for (j = 0; j <= i; j++)
          printf(1, "\t");
        printf(1, "%d\n", tc[i].tick_share);

        exit();
    }
  }

  // Wait for all chuldren to exit
  int children_left = NUM_CHILDREN;
  while (children_left > 0) {
    wait();
    children_left--;
  }
}
 
int
main(int argc, char *argv[])
{
  if (argc != 3) {
    printf(2, "usage: %d trials duration\n", argv[0]);
    exit();
  }

  // Read user input
  TRIALS = atoi(argv[1]);
  DURATION = atoi(argv[2]);
  if (TRIALS == 0 || DURATION == 0) {
    printf(2, "invalid number input\n");
    exit();
  }

  int i;
  printf(1, "%d trials: running %d processes for %d ticks each\n", TRIALS, NUM_CHILDREN, DURATION);

  // Sum total tickets
  for (i = 0; i < NUM_CHILDREN; i++)
    total_tickets += (i+1)*TICKET_MULTIPLIER;

  // Print child numbers header
  printf(1, "child:\t");
  for (i = 0; i < NUM_CHILDREN; i++) {
    printf(1, "#%d", i);
    if (i == NUM_CHILDREN-1) printf(1, "\n");
    else printf(1, "\t");
  }

  // Print ideal header
  printf(1, "ideal:\t");
  for (i = 0; i < NUM_CHILDREN; i++) {
    int tickets = (i+1)*TICKET_MULTIPLIER;
    int ideal_ticks = ((float)tickets/(float)total_tickets) * DURATION;
    printf(1, "%d", ideal_ticks);
    if (i == NUM_CHILDREN-1) printf(1, "\n");
    else printf(1, "\t");
  }

  // Print trial results
  for (i = 0; i < TRIALS; i++) {
    run_trial(i);
  }
  
  exit();
}