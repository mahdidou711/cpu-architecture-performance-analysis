#define _POSIX_C_SOURCE 200809L
#include "tsc.h"
#include <stdio.h>
#include <time.h>
#include <inttypes.h>

static double seconds(struct timespec t) { return t.tv_sec + t.tv_nsec * 1e-9; }
int main(void)
{
  if (!tsc_supported()) { fprintf(stderr, "RDTSCP/SSE2 unavailable\n"); return 1; }
  unsigned a, b, c, d;
  if (__get_cpuid_count(0x1a, 0, &a, &b, &c, &d))
    fprintf(stderr, "cpuid_1a_core_type=0x%x (Intel: 0x40 Core, 0x20 Atom)\n", a >> 24);
  if (__get_cpuid_count(0x15, 0, &a, &b, &c, &d))
    fprintf(stderr, "cpuid_15 denominator=%u numerator=%u crystal_hz=%u tsc_hz=%.0f\n", a, b, c, a ? (double)c * b / a : 0);
  puts("function,sample,ticks,denominator,ticks_per_iteration,aux_start,aux_stop,valid");
  for (int i = 0; i < 1001; i++) {
    uint64_t start = start_timer(), stop = stop_timer();
    printf("EMPTY,%d,%" PRIu64 ",1,%" PRIu64 ",%u,%u,%d\n", i, stop-start, stop-start, tsc_aux_start, tsc_aux_stop, tsc_aux_start == tsc_aux_stop);
  }
  /* Only a coarse cross-check: monotonic reads bracket TSC, no calibration claim. */
  for (int i = 0; i < 5; i++) {
    struct timespec before, after, pause = {0, 20000000};
    clock_gettime(CLOCK_MONOTONIC_RAW, &before);
    uint64_t start = start_timer();
    if (nanosleep(&pause, NULL)) return 2;
    uint64_t stop = stop_timer();
    clock_gettime(CLOCK_MONOTONIC_RAW, &after);
    fprintf(stderr, "crosscheck sample=%d elapsed_s=%.9f ticks=%" PRIu64 " estimate_hz=%.0f aux_start=%u aux_stop=%u\n", i, seconds(after)-seconds(before), stop-start, (stop-start)/(seconds(after)-seconds(before)), tsc_aux_start, tsc_aux_stop);
  }
  return 0;
}
