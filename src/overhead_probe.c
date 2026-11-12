/* Mise à l'échelle du coût d'instrumentation avec le harnais tsc.h.
   Noyau : chaîne de L additions dépendantes (latence connue, aucun accès mémoire). La régression
   ticks = a + b*L estime l'ordonnée à l'origine ; aucune soustraction n'est appliquée aux résultats.
   Usage : overhead_probe SAMPLES  -> CSV : L,sample,ticks,aux_start,aux_stop */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "tsc.h"

#define ADD8 "add %[y], %[x]\n\t"
static uint64_t chain_add(uint64_t x, uint64_t y, long iters)
{ /* 8 adds dépendants par itération */
  __asm__ volatile("1:\n\t" ADD8 ADD8 ADD8 ADD8 ADD8 ADD8 ADD8 ADD8 "dec %[n]\n\tjnz 1b"
                   : [x] "+r"(x), [n] "+r"(iters) : [y] "r"(y) : "cc");
  return x;
}
static volatile uint64_t sink;
int main(int argc, char **argv)
{
  if (argc < 2 || !tsc_supported()) return 2;
  int samples = atoi(argv[1]);
  static const long iters[] = {1, 4, 16, 64, 256, 1024, 4096, 16384, 65536, 262144};
  uint64_t x = 1;
  puts("adds,sample,ticks,aux_start,aux_stop");
  for (unsigned k = 0; k < sizeof iters / sizeof iters[0]; k++) {
    for (int s = 0; s < 3; s++) x = chain_add(x, 3, iters[k]); /* échauffement hors mesure */
    for (int s = 0; s < samples; s++) {
      uint64_t t0 = start_timer(); x = chain_add(x, 3, iters[k]); uint64_t t1 = stop_timer();
      printf("%ld,%d,%" PRIu64 ",%u,%u\n", iters[k] * 8, s, t1 - t0, tsc_aux_start, tsc_aux_stop);
    }
  }
  sink = x;
  return 0;
}
