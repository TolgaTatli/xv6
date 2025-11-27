#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "pstat.h"  // LOTTERY SCHEDULER: Process istatistikleri için
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//   settickets(10);  for 10 tickets
// Get ticket count from user (with argint)
// Check if number is valid (must be at least 1)
// Update current process's tickets field

uint64
sys_settickets(void)
{
  int tickets;
  
  // STEP 1: Get ticket count from user
  // argint(0, &tickets) -> Write first parameter (index 0) to tickets variable
  argint(0, &tickets);
  
  // STEP 2: Check if ticket count is valid
  // Must have at least 1 ticket
  // Why? 0 tickets = process never selected = STARVATION!
  if(tickets < 1)
    return -1;  // Invalid ticket count
  
  // STEP 3: Update current process's ticket count
  struct proc *p = myproc();  // Currently running process
  
  acquire(&p->lock);  // Acquire lock (safety)
  p->tickets = tickets;  // Set ticket count
  release(&p->lock);  // Release lock
  
  return 0;  // Success!
}

// SYS_GETPINFO - Get information about all processes
//
// USAGE:
//   struct pstat info;
//   getpinfo(&info);
//   printf("Process 0: %d tickets\n", info.tickets[0]);
//
// HOW IT WORKS:
// 1. Get pstat structure address from user
// 2. Loop through all processes, collect information
// 3. Copy information to user space
//
// WHY NEEDED:
// - For test programs
// - To see process status
// - To verify lottery scheduler works correctly
//
uint64
sys_getpinfo(void)
{
  uint64 addr;  // Address provided by user
  struct pstat pstat;  // Structure to hold process information
  
  // STEP 1: Get pstat structure address from user
  // argaddr(0, &addr) -> Get first parameter (of type struct pstat*)
  argaddr(0, &addr);
  
  // STEP 2: Collect information from all processes
  // Access proc[] array using extern declaration
  extern struct proc proc[];
  struct proc *p;
  int i = 0;
  
  // Loop through all slots in proc[] array
  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);  // Acquire lock for each process
    
    // Is this slot in use?
    // UNUSED = 0, other states (USED, RUNNABLE, RUNNING...) = in use
    pstat.inuse[i] = (p->state != UNUSED);
    
    // Copy process information
    pstat.tickets[i] = p->tickets;  // Ticket count
    pstat.pid[i] = p->pid;          // Process ID
    pstat.ticks[i] = p->ticks;      // CPU time
    
    release(&p->lock);  // Release lock
    i++;
  }
  
  // STEP 3: Copy information to user space
  // copyout(pagetable, dst, src, len)
  //   pagetable: User's page table
  //   dst: Destination address in user space
  //   src: Source address in kernel
  //   len: Number of bytes to copy
  //
  // Why copyout? Because kernel and user are in different memory spaces!
  // We can't use direct memcpy, must copy through page table
  if(copyout(myproc()->pagetable, addr, (char *)&pstat, sizeof(pstat)) < 0)
    return -1;  // Copy error
  
  return 0;  // Success!
}

// SYS_GETSYSCALLCOUNT - Get syscall count for current process
//
// USAGE:
//   int count = getsyscallcount(SYS_fork);
//   printf("fork called %d times\n", count);
//
// HOW IT WORKS:
// 1. Get syscall number from user (which syscall to query)
// 2. Validate the syscall number (must be valid)
// 3. Return the count from current process's syscall_count array
//
// WHY NEEDED:
// - For testing and debugging
// - To see how many times each syscall was used
// - To analyze program behavior
//
uint64
sys_getsyscallcount(void)
{
  int syscall_num;
  
  // STEP 1: Get the syscall number argument
  // User passes which syscall they want to query (e.g., SYS_fork)
  argint(0, &syscall_num);
  
  // STEP 2: Validate syscall number
  // Must be between 0 and 24 (we have 25 syscalls: 0-24)
  if(syscall_num < 0 || syscall_num >= 25)
    return -1;
  
  // STEP 3: Return the count for this syscall
  // Access current process's syscall_count array
  struct proc *p = myproc();
  return p->syscall_count[syscall_num];
}
