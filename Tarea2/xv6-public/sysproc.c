#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
#include "pstat.h"
int
sys_settickets(void) {
    int n;

    if(argint(0, &n) < 0 || n < 1)
        return -1;

    acquire(&ptable.lock);
    myproc()->tickets = n;
    release(&ptable.lock);
    return 0;
}

int
sys_getpinfo(void) {
    struct pstat* ps;
    struct proc* p;

    if(argint(0, (int*)(&ps)) < 0)
		return -1;

    acquire(&ptable.lock);
    int i = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        ps->inuse[i] = p->inuse;
        ps->tickets[i] = p->tickets;
        ps->pid[i] = p->pid;
        ps->ticks[i] = p->ticks;
        i++;
    }
    release(&ptable.lock);
    return 0;
}