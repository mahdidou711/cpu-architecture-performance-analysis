/* Current logical-processor identity (run under taskset).
   Output: one JSON line. No system writes; CPUID and RDTSCP only. */
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include "tsc.h"

int main(void)
{
  unsigned a, b, c, d, aux;
  char brand[49] = {0};
  if (!tsc_supported()) { fprintf(stderr, "RDTSCP/SSE2 unavailable\n"); return 1; }
  /* Every leaf is queried through __get_cpuid_count, which checks the
     maximum supported leaf; unavailable leaves are omitted from the JSON. */
  if (__get_cpuid_max(0x80000000u, 0) >= 0x80000004u)
    for (unsigned i = 0; i < 3; i++) {
      __cpuid(0x80000002u + i, a, b, c, d);
      memcpy(brand + 16 * i, &a, 4); memcpy(brand + 16 * i + 4, &b, 4);
      memcpy(brand + 16 * i + 8, &c, 4); memcpy(brand + 16 * i + 12, &d, 4);
    }
  read_timer(&aux);
  printf("{\"sched_getcpu\":%d,\"tsc_aux\":%u,\"brand\":\"%s\"", sched_getcpu(), aux, brand);
  if (__get_cpuid_count(0x1a, 0, &a, &b, &c, &d))
    printf(",\"cpuid_1a_eax\":\"0x%x\",\"core_type_code\":\"0x%x\",\"native_model_id\":\"0x%x\"", a, a >> 24, a & 0xffffff);
  if (__get_cpuid_count(0x15, 0, &a, &b, &c, &d))
    printf(",\"cpuid_15\":{\"den\":%u,\"num\":%u,\"crystal_hz\":%u}", a, b, c);
  if (__get_cpuid_count(0x16, 0, &a, &b, &c, &d))
    printf(",\"cpuid_16_mhz\":{\"base\":%u,\"max\":%u,\"bus\":%u}", a & 0xffff, b & 0xffff, c & 0xffff);
  if (__get_cpuid_count(6, 0, &a, &b, &c, &d))
    printf(",\"cpuid_6_eax\":\"0x%x\"", a);
  if (__get_cpuid_count(7, 0, &a, &b, &c, &d))
    printf(",\"cpuid_7_edx_hybrid_bit15\":%u", (d >> 15) & 1);
  if (__get_cpuid_count(0x80000007u, 0, &a, &b, &c, &d))
    printf(",\"invariant_tsc\":%u", (d >> 8) & 1);
  printf(",\"caches\":[");
  for (unsigned i = 0;; i++) {
    if (!__get_cpuid_count(4, i, &a, &b, &c, &d)) break;
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
