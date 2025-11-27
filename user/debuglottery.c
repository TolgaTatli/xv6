#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pstat.h"
#include "user/user.h"

// DEBUG TEST: Scheduler test

int
main(int argc, char *argv[])
{
  printf("LOTTERY SCHEDULER DEBUG TEST\n");
  
  struct pstat pstat;
  
  // İlk durumu al
  if(getpinfo(&pstat) == 0) {
    printf("\nŞu anki process'ler:\n");
    printf("----------------------------\n");
    for(int i = 0; i < 64; i++) {
      if(pstat.inuse[i]) {
        printf("PID=%d, Tickets=%d, Ticks=%d\n", 
               pstat.pid[i], pstat.tickets[i], pstat.ticks[i]);
      }
    }
  }
  
  printf("Test: settickets() çalışıyor mu?\n");
  
  printf("Mevcut PID: %d\n", getpid());
  printf("Bilet ayarlamadan önce tickets değeri kontrol ediliyor...\n");
  
  if(getpinfo(&pstat) == 0) {
    int my_pid = getpid();
    for(int i = 0; i < 64; i++) {
      if(pstat.pid[i] == my_pid) {
        printf("  -> Şu anki tickets: %d\n", pstat.tickets[i]);
        break;
      }
    }
  }
  
  printf("\nsettickets(50) çağrılıyor...\n");
  if(settickets(50) == 0) {
    printf("✅ settickets(50) başarılı!\n");
  } else {
    printf("❌ settickets(50) BAŞARISIZ!\n");
  }
  
  if(getpinfo(&pstat) == 0) {
    int my_pid = getpid();
    for(int i = 0; i < 64; i++) {
      if(pstat.pid[i] == my_pid) {
        printf("  -> Yeni tickets: %d\n", pstat.tickets[i]);
        if(pstat.tickets[i] == 50) {
          printf("✅ Bilet sayısı başarıyla güncellendi!\n");
        } else {
          printf("❌ Bilet sayısı güncellenemedi! Hala %d\n", pstat.tickets[i]);
        }
        break;
      }
    }
  }
  
  printf("\n==============================================\n");
  printf("Test: Fork çocuğa bilet miras kalıyor mu?\n");
  printf("==============================================\n");
  
  int pid = fork();
  if(pid == 0) {
    // Çocuk process
    printf("Çocuk process (PID=%d) oluşturuldu\n", getpid());
    
    if(getpinfo(&pstat) == 0) {
      int my_pid = getpid();
      for(int i = 0; i < 64; i++) {
        if(pstat.pid[i] == my_pid) {
          printf("  -> Çocuğun tickets: %d (parent'tan miras)\n", pstat.tickets[i]);
          if(pstat.tickets[i] == 50) {
            printf("✅ Fork miras çalışıyor!\n");
          } else {
            printf("❌ Fork miras ÇALIŞMIYOR! Tickets=%d (beklenen: 50)\n", pstat.tickets[i]);
          }
          break;
        }
      }
    }
    exit(0);
  }
  
  wait(0);
  
  printf("\n==============================================\n");
  printf("Debug testi tamamlandı.\n");
  printf("==============================================\n");
  
  exit(0);
}
