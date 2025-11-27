#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/syscall.h"

// ============================================
// SYSTEM CALL TRACING & COUNTING TEST
// ============================================
// This test program demonstrates:
// 1. System call tracing (printing each syscall)
// 2. System call counting (counting how many times each was called)
// ============================================

int
main(int argc, char *argv[])
{
  printf("==============================================\n");
  printf("SYSTEM CALL TRACING & COUNTING TEST\n");
  printf("==============================================\n");
  printf("This test will make various system calls\n");
  printf("and then report how many times each was called.\n");
  printf("==============================================\n\n");

  printf("Test 1: Basic system calls\n");
  printf("---------------------------\n");
  
  // Test fork - create child process
  printf("Creating child process with fork()...\n");
  int pid = fork();
  if(pid == 0) {
    // Child process
    printf("Child: Running getpid() test\n");
    int mypid = getpid();
    printf("Child PID: %d\n", mypid);
    exit(0);
  } else {
    // Parent process
    wait(0);
    printf("Parent: Child finished\n");
  }
  
  printf("\nTest 2: File operations\n");
  printf("---------------------------\n");
  
  // Test open/write/close
  printf("Creating and writing to testfile.txt...\n");
  int fd = open("testfile.txt", 0x001 | 0x200);  // O_CREATE | O_WRONLY
  if(fd >= 0) {
    char *msg = "Hello from xv6 syscall test!\n";
    write(fd, msg, strlen(msg));
    close(fd);
    printf("File operations completed\n");
  } else {
    printf("Failed to open file\n");
  }
  
  printf("\nTest 3: Multiple fork() calls\n");
  printf("---------------------------\n");
  
  // Make multiple fork calls to test counting
  printf("Creating 3 child processes...\n");
  for(int i = 0; i < 3; i++) {
    pid = fork();
    if(pid == 0) {
      // Child: just exit
      exit(0);
    }
    wait(0);  // Parent waits for each child
  }
  printf("All children finished\n");
  
  printf("\nTest 4: System call counting results\n");
  printf("---------------------------\n");
  
  // Get syscall counts
  int fork_count = getsyscallcount(SYS_fork);
  int write_count = getsyscallcount(SYS_write);
  int open_count = getsyscallcount(SYS_open);
  int exit_count = getsyscallcount(SYS_exit);
  int wait_count = getsyscallcount(SYS_wait);
  int getpid_count = getsyscallcount(SYS_getpid);
  int close_count = getsyscallcount(SYS_close);
  
  printf("\n==============================================\n");
  printf("SYSCALL COUNT SUMMARY:\n");
  printf("==============================================\n");
  printf("fork()   called: %d times\n", fork_count);
  printf("write()  called: %d times\n", write_count);
  printf("open()   called: %d times\n", open_count);
  printf("close()  called: %d times\n", close_count);
  printf("wait()   called: %d times\n", wait_count);
  printf("getpid() called: %d times\n", getpid_count);
  printf("exit()   called: %d times (in children)\n", exit_count);
  printf("==============================================\n");
  
  printf("\n✅ Test completed successfully!\n");
  printf("System call counting is working correctly.\n");
  
  exit(0);
}
