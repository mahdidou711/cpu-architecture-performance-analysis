/* Identité du processeur logique courant (à lancer sous taskset).
   Sortie : une ligne JSON. Aucune écriture système ; CPUID et RDTSCP seulement. */
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include "tsc.h"

int main(void)
{
  unsigned a, b, c, d, aux;
  char brand[49] = {0};
  for (unsigned i = 0; i < 3; i++) {
    __cpuid(0x80000002u + i, a, b, c, d);
    memcpy(brand + 16 * i, &a, 4); memcpy(brand + 16 * i + 4, &b, 4);
    memcpy(brand + 16 * i + 8, &c, 4); memcpy(brand + 16 * i + 12, &d, 4);
  }
  read_timer(&aux);
  printf("{\"sched_getcpu\":%d,\"tsc_aux\":%u,\"brand\":\"%s\"", sched_getcpu(), aux, brand);
  __cpuid_count(0x1a, 0, a, b, c, d);
  printf(",\"cpuid_1a_eax\":\"0x%x\",\"core_type_code\":\"0x%x\",\"native_model_id\":\"0x%x\"", a, a >> 24, a & 0xffffff);
  if (__get_cpuid_count(0x15, 0, &a, &b, &c, &d))
    printf(",\"cpuid_15\":{\"den\":%u,\"num\":%u,\"crystal_hz\":%u}", a, b, c);
  if (__get_cpuid_count(0x16, 0, &a, &b, &c, &d))
    printf(",\"cpuid_16_mhz\":{\"base\":%u,\"max\":%u,\"bus\":%u}", a & 0xffff, b & 0xffff, c & 0xffff);
  __cpuid_count(6, 0, a, b, c, d);
  printf(",\"cpuid_6_eax\":\"0x%x\"", a);
  __cpuid_count(7, 0, a, b, c, d);
  printf(",\"cpuid_7_edx_hybrid_bit15\":%u", (d >> 15) & 1);
  __cpuid_count(0x80000007u, 0, a, b, c, d);
  printf(",\"invariant_tsc\":%u", (d >> 8) & 1);
  printf(",\"caches\":[");
  for (unsigned i = 0;; i++) {
    __cpuid_count(4, i, a, b, c, d);
    unsigned type = a & 31;
    if (!type) break;
    unsigned level = (a >> 5) & 7, ways = ((b >> 22) & 0x3ff) + 1, parts = ((b >> 12) & 0x3ff) + 1,
             line = (b & 0xfff) + 1, sets = c + 1;
    printf("%s{\"level\":%u,\"type\":%u,\"ways\":%u,\"line\":%u,\"sets\":%u,\"bytes\":%u,\"max_threads_sharing\":%u}",
           i ? "," : "", level, type, ways, line, sets, ways * parts * line * sets, ((a >> 14) & 0xfff) + 1);
  }
  printf("]}\n");
  return 0;
}
