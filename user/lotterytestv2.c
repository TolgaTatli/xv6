#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pstat.h"
#include "user/user.h"

// Simple test: 2 processes, one with 10 tickets, other with 1 ticket
// Check tick ratio at the end

int
main(int argc, char *argv[])
{
  printf("==============================================\n");
  printf("LOTTERY SCHEDULER SIMPLE TEST\n");
  printf("==============================================\n");
  printf("2 Processes: A=10 tickets, B=1 ticket\n");
  printf("Expected: A should get ~10x more CPU\n");
  printf("==============================================\n\n");
  
  int pid1 = fork();
  if(pid1 == 0) {
    // Process A: 10 tickets
    settickets(10);
    
    // Infinite loop - user will stop with Ctrl+C
    while(1) {
      // CPU intensive work
      for(int i = 0; i < 1000000; i++);
    }
  }
  
  int pid2 = fork();
  if(pid2 == 0) {
    // Process B: 1 ticket
    settickets(1);
    
    // Infinite loop
    while(1) {
      // CPU intensive work
      for(int i = 0; i < 1000000; i++);
    }
  }
  
  // Main process: wait 5 seconds, then report
  printf("Test running... (5 seconds)\n");
  printf("PID A=%d (10 tickets), PID B=%d (1 ticket)\n\n", pid1, pid2);
  
  pause(500);  // Wait 5 seconds (500 ticks)
  
  // Get results
  struct pstat pstat;
  if(getpinfo(&pstat) == 0) {
    int ticks_a = 0, ticks_b = 0;
    
    for(int i = 0; i < 64; i++) {
      if(pstat.pid[i] == pid1) ticks_a = pstat.ticks[i];
      if(pstat.pid[i] == pid2) ticks_b = pstat.ticks[i];
    }
    
    printf("==============================================\n");
    printf("RESULTS (after 5 seconds):\n");
    printf("==============================================\n");
    printf("Process A (10 tickets): %d ticks\n", ticks_a);
    printf("Process B (1 ticket):   %d ticks\n", ticks_b);
    
    if(ticks_b > 0) {
      int ratio = (ticks_a * 100) / ticks_b;
      printf("\nRatio: A/B = %d%% (expected: 1000%%)\n", ratio);
      
      if(ratio >= 800 && ratio <= 1200) {
        printf("\n✅ TEST PASSED!\n");
        printf("Lottery scheduler works correctly!\n");
      } else if(ratio >= 500 && ratio <= 1500) {
        printf("\n⚠️  Ratio is in acceptable range.\n");
        printf("Longer test might give better results.\n");
      } else {
        printf("\n❌ WARNING: Not in expected range!\n");
      }
    }
    printf("==============================================\n");
  }
  
  // Kill children
  kill(pid1);
  kill(pid2);
  wait(0);
  wait(0);
  
  printf("\nTest completed.\n");
  exit(0);
}
