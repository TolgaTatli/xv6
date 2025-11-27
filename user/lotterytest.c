#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pstat.h"
#include "user/user.h"

// =============================================================================
// LOTTERY SCHEDULER TEST PROGRAMI
// =============================================================================
//
// Bu program lottery scheduler'ın düzgün çalışıp çalışmadığını test eder
//
// TEST SENARYOSU:
// - 3 process oluşturur
// - Process A: 30 bilet (3:2:1 oranında en çok bilet)
// - Process B: 20 bilet 
// - Process C: 10 bilet
//
// BEKLENEN SONUÇ:
// - A, toplam CPU zamanının ~%50'sini almalı (30/60)
// - B, toplam CPU zamanının ~%33'ünü almalı (20/60)
// - C, toplam CPU zamanının ~%17'sini almalı (10/60)
//
// =============================================================================

#define TICKETS_A 30
#define TICKETS_B 20
#define TICKETS_C 10
#define TEST_DURATION 5000  // Test süresi (döngü sayısı) - 1000'den 5000'e çıkardık

void
spin()
{
  // CPU yoğun iş yap (boş döngü)
  // Bu, process'in CPU kullanmasını sağlar
  int i, j;
  for(i = 0; i < 1000000; i++) {
    j = i * i;
    j = j + 1;
  }
}

int
main(int argc, char *argv[])
{
  int pid_a, pid_b, pid_c;
  struct pstat pstat;
  
  printf("==============================================\n");
  printf("LOTTERY SCHEDULER TEST PROGRAMI\n");
  printf("==============================================\n");
  printf("Test Senaryosu: 3 Process (30:20:10 bilet)\n");
  printf("Beklenen: A %%50, B %%33, C %%17 CPU zamani\n");
  printf("==============================================\n\n");
  
  // ==========================================================================
  // PROCESS A: 30 BİLET
  // ==========================================================================
  pid_a = fork();
  if(pid_a == 0) {
    // Çocuk process A
    settickets(TICKETS_A);  // 30 bilet ayarla
    
    int my_pid = getpid();
    printf("[Process A] PID=%d, Tickets=%d BAŞLADI\n", my_pid, TICKETS_A);
    
    // CPU yoğun iş yap
    for(int i = 0; i < TEST_DURATION; i++) {
      spin();
      
      // Çıktı karışıklığını önlemek için bu kısmı kaldırdık
      // Her 100 iterasyonda rapor etme yerine sadece sonunda rapor edeceğiz
    }
    
    printf("[Process A] PID=%d TAMAMLANDI\n", my_pid);
    exit(0);
  }
  
  // ==========================================================================
  // PROCESS B: 20 BİLET
  // ==========================================================================
  pid_b = fork();
  if(pid_b == 0) {
    // Çocuk process B
    settickets(TICKETS_B);  // 20 bilet ayarla
    
    int my_pid = getpid();
    printf("[Process B] PID=%d, Tickets=%d BAŞLADI\n", my_pid, TICKETS_B);
    
    // CPU yoğun iş yap
    for(int i = 0; i < TEST_DURATION; i++) {
      spin();
    }
    
    printf("[Process B] PID=%d TAMAMLANDI\n", my_pid);
    exit(0);
  }
  
  // ==========================================================================
  // PROCESS C: 10 BİLET
  // ==========================================================================
  pid_c = fork();
  if(pid_c == 0) {
    // Çocuk process C
    settickets(TICKETS_C);  // 10 bilet ayarla
    
    int my_pid = getpid();
    printf("[Process C] PID=%d, Tickets=%d BAŞLADI\n", my_pid, TICKETS_C);
    
    // CPU yoğun iş yap
    for(int i = 0; i < TEST_DURATION; i++) {
      spin();
    }
    
    printf("[Process C] PID=%d TAMAMLANDI\n", my_pid);
    exit(0);
  }
  
  // ==========================================================================
  // ANA PROCESS: ÇOCUKLARI BEKLE VE SONUÇLARI RAPORLA
  // ==========================================================================
  
  // Çocukların tamamlanmasını bekle
  wait(0);
  wait(0);
  wait(0);
  
  printf("\n==============================================\n");
  printf("TEST TAMAMLANDI - SONUÇLAR\n");
  printf("==============================================\n");
  
  // Son durumu al
  if(getpinfo(&pstat) == 0) {
    int ticks_a = 0, ticks_b = 0, ticks_c = 0;
    
    // Her process'in toplam tick'ini bul
    for(int i = 0; i < 64; i++) {
      if(pstat.inuse[i]) {
        if(pstat.pid[i] == pid_a) ticks_a = pstat.ticks[i];
        if(pstat.pid[i] == pid_b) ticks_b = pstat.ticks[i];
        if(pstat.pid[i] == pid_c) ticks_c = pstat.ticks[i];
      }
    }
    
    int total_ticks = ticks_a + ticks_b + ticks_c;
    
    if(total_ticks > 0) {
      printf("\nProcess A (30 tickets): %d ticks (%%%d)\n", 
             ticks_a, (ticks_a * 100) / total_ticks);
      printf("Process B (20 tickets): %d ticks (%%%d)\n", 
             ticks_b, (ticks_b * 100) / total_ticks);
      printf("Process C (10 tickets): %d ticks (%%%d)\n", 
             ticks_c, (ticks_c * 100) / total_ticks);
      printf("Toplam: %d ticks\n", total_ticks);
      
      printf("\n");
      printf("BEKLENEN ORANLAR:\n");
      printf("  A: %%50 (30/60)\n");
      printf("  B: %%33 (20/60)\n");
      printf("  C: %%17 (10/60)\n");
      
      // Basit bir kontrol
      int ratio_ab = (ticks_a * 100) / (ticks_b > 0 ? ticks_b : 1);
      int ratio_ac = (ticks_a * 100) / (ticks_c > 0 ? ticks_c : 1);
      int ratio_bc = (ticks_b * 100) / (ticks_c > 0 ? ticks_c : 1);
      
      printf("\nGERÇEK ORANLAR:\n");
      printf("  A/B = %d%% (beklenen: 150%%)\n", ratio_ab);
      printf("  A/C = %d%% (beklenen: 300%%)\n", ratio_ac);
      printf("  B/C = %d%% (beklenen: 200%%)\n", ratio_bc);
      
      // Başarı kontrolü (kabaca)
      if(ratio_ab >= 130 && ratio_ab <= 170 &&
         ratio_ac >= 250 && ratio_ac <= 350 &&
         ratio_bc >= 170 && ratio_bc <= 230) {
        printf("\n✅ TEST BAŞARILI! Lottery scheduler doğru çalışıyor!\n");
      } else {
        printf("\n⚠️  UYARI: Oranlar beklenen aralıkta değil. Daha uzun test gerekebilir.\n");
      }
    }
  } else {
    printf("HATA: getpinfo() çağrısı başarısız!\n");
  }
  
  printf("==============================================\n");
  
  exit(0);
}
