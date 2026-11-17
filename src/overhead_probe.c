/* TSC instrumentation-overhead scaling with the tsc.h harness.
   Kernel: a chain of L dependent additions (known latency, no memory access). The regression
   ticks = a + b*L estimates the intercept; no subtraction is applied to the results.
   Usage: overhead_probe SAMPLES -> CSV: L,sample,ticks,aux_start,aux_stop */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "tsc.h"

#define ADD8 "add %[y], %[x]\n\t"
static uint64_t chain_add(uint64_t x, uint64_t y, long iters)
{ /* 8 dependent adds per iteration */
  __asm__ volatile("1:\n\t" ADD8 ADD8 ADD8 ADD8 ADD8 ADD8 ADD8 ADD8 "dec %[n]\n\tjnz 1b"
                   : [x] "+r"(x), [n] "+r"(iters) : [y] "r"(y) : "cc");
  return x;
}
static volatile uint64_t sink;
int main(int argc, char **argv)
{
  if (argc < 2) { fprintf(stderr, "usage: %s SAMPLES   (SAMPLES >= 1)\n", argv[0]); return 2; }
  if (!tsc_supported()) { fprintf(stderr, "RDTSCP/SSE2 unavailable\n"); return 2; }
  char *end;
  long parsed = strtol(argv[1], &end, 10);
  if (*argv[1] == '\0' || *end != '\0' || parsed < 1 || parsed > 1000000) {
    fprintf(stderr, "invalid SAMPLES '%s' (expected an integer in 1..1000000)\n", argv[1]);
    return 2;
  }
  int samples = (int)parsed;
  static const long iters[] = {1, 4, 16, 64, 256, 1024, 4096, 16384, 65536, 262144};
  uint64_t x = 1;
  puts("adds,sample,ticks,aux_start,aux_stop");
  for (unsigned k = 0; k < sizeof iters / sizeof iters[0]; k++) {
    for (int s = 0; s < 3; s++) x = chain_add(x, 3, iters[k]); /* Warm-up outside the timed interval. */
    for (int s = 0; s < samples; s++) {
      uint64_t t0 = start_timer(); x = chain_add(x, 3, iters[k]); uint64_t t1 = stop_timer();
      printf("%ld,%d,%" PRIu64 ",%u,%u\n", iters[k] * 8, s, t1 - t0, tsc_aux_start, tsc_aux_stop);
    }
  }
  sink = x;
  return 0;
}
