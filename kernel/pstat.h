#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

// =============================================================================
// PSTAT YAPISI - Process İstatistikleri
// =============================================================================
//
// Bu yapı, tüm process'lerin durumunu saklar
// Test programları bu yapıyı kullanarak:
// - Hangi process'ler çalışıyor?
// - Kaç biletleri var?
// - Ne kadar CPU zamanı aldılar?
// gibi bilgilere erişir
//
// NEDEN DIZI?
// - Sistem en fazla NPROC (64) process'e sahip
// - Her process için bilgi tutuyoruz
// - inuse[i] -> i numaralı slot kullanımda mı?
// - tickets[i] -> i numaralı slot'taki process'in bilet sayısı
//
struct pstat {
  int inuse[NPROC];   // Process kullanımda mı? (1 = evet, 0 = hayır)
  int tickets[NPROC]; // Her process'in bilet sayısı
  int pid[NPROC];     // Her process'in ID'si
  int ticks[NPROC];   // Her process'in aldığı CPU zamanı (tick cinsinden)
};

#endif // _PSTAT_H_
