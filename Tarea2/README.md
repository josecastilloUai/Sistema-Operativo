# Lottery Scheduler

Este proyecto implementa un planificador de lotería en el sistema operativo. El planificador asigna a cada proceso un número de boletos y, en cada conmutación de contexto, selecciona un proceso ganador al azar para ejecutarse. La probabilidad de que un proceso gane depende del número de boletos asignados.

## Archivos del proyecto

1. `utils.h` y `utils.c`: Contienen una función para generar números pseudoaleatorios en un rango dado.

     ```c
     int random(int max);
     ```
     ```c
     #include "utils.h"

    unsigned long next = 1;
    int random(int max)
    {
        if (max == 0) return 0;
        next = next * 1103515245 + 12345;
        int rand = ((unsigned)(next/65536) % 32768);
        return (rand % max + 1) - 1;
    }
     ```

2. `test.c`: Es el programa principal que ejecuta múltiples pruebas para determinar cómo se distribuye la carga de la CPU entre varios procesos. Utiliza el planificador de lotería implementado.
    ```c
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

    ```

3. `pstat.h`: Define la estructura `pstat`, que se utiliza para almacenar información sobre los procesos en ejecución.
    ```c
    #ifndef _PSTAT_H_
    #define _PSTAT_H_

    #include "param.h"

    struct pstat {
    int inuse[NPROC];   // whether this slot of the process table is in use (1 or 0)
    int tickets[NPROC]; // the number of tickets this process has
    int pid[NPROC];     // the PID of each process 
    int ticks[NPROC];   // the number of ticks each process has accumulated 
    };
    ```

4. `ps.c`: Un programa que muestra información sobre los procesos en ejecución utilizando la estructura `pstat`.
    ```c
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
    ```

## Modificaciones en los archivos

### Archivos relacionados con el sistema operativo

- `usys.S`: Se agregaron las llamadas al sistema `settickets` y `getpinfo`. 
    ```c
    SYSCALL(fork)
    SYSCALL(exit)
    SYSCALL(wait)
    SYSCALL(pipe)
    SYSCALL(read)
    SYSCALL(write)
    SYSCALL(close)
    SYSCALL(kill)
    SYSCALL(exec)
    SYSCALL(open)
    SYSCALL(mknod)
    SYSCALL(unlink)
    SYSCALL(fstat)
    SYSCALL(link)
    SYSCALL(mkdir)
    SYSCALL(chdir)
    SYSCALL(dup)
    SYSCALL(getpid)
    SYSCALL(sbrk)
    SYSCALL(sleep)
    SYSCALL(uptime)
    SYSCALL(settickets)
    SYSCALL(getpinfo)
    ```

- `user.h`: Se incluyó la declaración de las llamadas al sistema `settickets` y `getpinfo`. ademas de añadir  al inicio #include "pstat.h"
    ```c
    #include "pstat.h"

    struct stat;
    struct rtcdate;

    // system calls
    int fork(void);
    int exit(void) __attribute__((noreturn));
    int wait(void);
    int pipe(int*);
    int write(int, const void*, int);
    int read(int, void*, int);
    int close(int);
    int kill(int);
    int exec(char*, char**);
    int open(const char*, int);
    int mknod(const char*, short, short);
    int unlink(const char*);
    int fstat(int fd, struct stat*);
    int link(const char*, const char*);
    int mkdir(const char*);
    int chdir(const char*);
    int dup(int);
    int getpid(void);
    char* sbrk(int);
    int sleep(int);
    int uptime(void);
    int settickets(int);
    int getpinfo(struct pstat *);

    ```

- `sysproc.c`: Se implementaron las funciones del sistema `settickets` y `getpinfo`.
    ```c
    //Despues de  int sys_uptime(void) {}
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
    ```

- `syscall.h`: Se agregaron las definiciones de las llamadas al sistema `settickets` y `getpinfo`.
    ```c
    #define SYS_settickets 22
    #define SYS_getpinfo 23
    ```

- `syscall.c`: Se incluyeron los punteros a las funciones de las llamadas al sistema `settickets` y `getpinfo`.
    ```c
    extern int sys_chdir(void);
    // 
    extern int sys_settickets(void);
    extern int sys_getpinfo(void);

    static int (*syscalls[])(void) = {
    [SYS_fork]    sys_fork,
    //  
    [SYS_settickets] sys_settickets,
    [SYS_getpinfo]   sys_getpinfo,
    };

    ```
- `spinlock.h`: Se reemplazó el contenido del archivo por la definición de la estructura `spinlock`.
    ```c
    #ifndef _SPINLOCK_H_
    #define _SPINLOCK_H_

    // Mutual exclusion lock.
    struct spinlock {
    uint locked;       // Is the lock held?

    // For debugging:
    char *name;        // Name of lock.
    struct cpu *cpu;   // The cpu holding the lock.
    uint pcs[10];      // The call stack (an array of program counters)
                        // that locked the lock.
    };

    #endif
    ```


### Archivos relacionados con el planificador
- `proc.h`: Se agregaron campos a la estructura `proc` para mantener el número de boletos y los ticks acumulados por cada proceso. También se definió la estructura `ptable_type` que reemplaza a la anterior `ptable`.
    - Se coloca al principio de proc.h .
    ```c
    #ifndef _PROC_H_
    #define _PROC_H_

    #include "spinlock.h"
    ```
    - Al struct proc se le añade tickets ,ticks y inuse.
    ```c
    // Per-process state
    struct proc {
    uint sz;                     // Size of process memory (bytes)
    pde_t* pgdir;                // Page table
    char *kstack;                // Bottom of kernel stack for this process
    enum procstate state;        // Process state
    int pid;                     // Process ID
    struct proc *parent;         // Parent process
    struct trapframe *tf;        // Trap frame for current syscall
    struct context *context;     // swtch() here to run process
    void *chan;                  // If non-zero, sleeping on chan
    int killed;                  // If non-zero, have been killed
    struct file *ofile[NOFILE];  // Open files
    struct inode *cwd;           // Current directory
    char name[16];               // Process name (debugging)
    int tickets;                 // Number of lottery tickets this process has
    int ticks;                   // How many ticks this process has accumulated
    int inuse;                  // Process name (debugging)
    };
    ```
    - Al final de proc.h se añade lo siguente.
     ```c
    struct ptable_type {
	  struct spinlock lock;
	  struct proc proc[NPROC];
     };
     extern struct ptable_type ptable;

    #endif // _PROC_H_
    ```

- `proc.c`: Se realizaron modificaciones en varias partes del código, incluyendo la inicialización de los campos de número de boletos, ticks y uso en la estructura `proc`. Se implementó una nueva función `number_tickets` para calcular el número total de boletos en el sistema. También se modificó la función `scheduler` para implementar el planificador de lotería.
    - 1.- Hay que eliminar todo lo anterior de int nextpid = 1 y colocar lo siguiente.
    ```c
    #include "types.h"
    #include "defs.h"
    #include "param.h"
    #include "memlayout.h"
    #include "mmu.h"
    #include "x86.h"
    #include "proc.h"
    #include "utils.h"

    struct ptable_type ptable = {0};

    static struct proc *initproc;
    ```
    - 2.- En allocproc(void):
    ```c
    static struct proc*
    allocproc(void)
    {
        // Inicios del codigo...

        found:
        p->state = EMBRYO;
        p->pid = nextpid++;
        p->tickets = 100;
        p->inuse = 0;
        p->ticks = 0;

        release(&ptable.lock);

        // Resto del codigo...
    }
    ```
    - 3.-  En fork(void):
    ```c
    int
    fork(void)
    {
        // Inicios del codigo... 

        np->sz = curproc->sz;
        np->parent = curproc;
        *np->tf = *curproc->tf;
        np->tickets = curproc->tickets;
        np->ticks = 0;
        np->inuse = 0;

         // Resto del codigo...  
    }
    
    ```
    - 4.-  nueva función `number_tickets`
    ```c
    int
    number_tickets()
    {
        struct proc *p;
        int total_tickets = 0;

        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
            if(p->state == RUNNABLE || p->state == RUNNING) total_tickets += p->tickets;
        }
        return total_tickets;
    }
    ```
    - 5.-  Modificar scheduler:
    ```c
    void
    scheduler(void)
    {
    struct proc *p;
    struct cpu *c = mycpu();
    c->proc = 0;

    int counter, winner;
    counter = winner = 0;
    
    for(;;){
        // Enable interrupts on this processor.
        sti();

        acquire(&ptable.lock);

        counter = 0;
        winner = random(number_tickets());

        // Loop over process table looking for process to run.
        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->state != RUNNABLE)
            continue;

        counter += p->tickets;
        if (counter <= winner)
            continue;

        // Switch to chosen process.  It is the process's job
        // to release ptable.lock and then reacquire it
        // before jumping back to us.

        
        c->proc = p;
        switchuvm(p);
        p->state = RUNNING;

        int begin = ticks;
        p->inuse = 1;
        swtch(&(c->scheduler), p->context);
        p->inuse = 0;
        p->ticks += ticks - begin;

        switchkvm();

        // Process is done running for now.
        // It should have changed its p->state before coming back.
        cprintf("EL PROCESO %d SE ESTA EJECUTANDO EN LA CPU CON %d TICKETS.\n", p->pid, p->tickets);
        c->proc = 0;

        break;
        }
        release(&ptable.lock);

     }
    }
    ```
### Makefile

- Se agregaron los archivos `utils.o`, `_ps` y `_test` a las variables `OBJS` y `UPROGS`.
    ```c
    OBJS = \
        bio.o\
        .
        .
        utils.o\

    ```
    ```c
    UPROGS=\
        _cat\
        .
        .
        _ps\
        _test\
    ```

- Se incluyeron los archivos `ps.c` y `test.c` en la lista de archivos fuente adicionales.
     ```c
    EXTRA=\
        mkfs.c ulib.c user.h cat.c echo.c forktest.c grep.c kill.c\
        ln.c ls.c mkdir.c rm.c stressfs.c usertests.c wc.c zombie.c\
        printf.c umalloc.c\
        README dot-bochsrc *.pl toc.* runoff runoff1 runoff.list\
        .gdbinit.tmpl gdbutil\
        ln.c ls.c mkdir.c rm.c stressfs.c usertests.c wc.c zombie.c ps.c test.c\
        printf.c umalloc.c utils.c\
    ```

## Uso del proyecto

El programa `test` se utiliza para ejecutar pruebas y evaluar el comportamiento del planificador de lotería. Para compilar y ejecutar el proyecto, sigue estos pasos:

1. Compila el proyecto:

   ```sh
   make qumu
