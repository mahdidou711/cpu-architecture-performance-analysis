#ifndef TP1_TSC_H
#define TP1_TSC_H
#include <stdint.h>
#include <cpuid.h>
#if !defined(__x86_64__) && !defined(__i386__)
#error "This timer requires x86 with SSE2 and RDTSCP"
#endif

/* Single-threaded teaching harness. AUX is an OS tag, not a portable CPU ID. */
static unsigned tsc_aux_start, tsc_aux_stop;
static inline int tsc_supported(void)
{
  unsigned a, b, c, d;
  if (!__get_cpuid(1, &a, &b, &c, &d) || !(d & (1u << 26))) return 0;
  return __get_cpuid(0x80000001u, &a, &b, &c, &d) && (d & (1u << 27));
}

/* Intel: MFENCE drains earlier stores; RDTSCP waits for earlier execution
   and loads; LFENCE prevents later execution crossing the timestamp.
   "memory" additionally bounds GCC's memory operations. Both endpoints
   use the same sequence, exposing AUX for migration rejection. */
static inline uint64_t read_timer(unsigned *aux)
{
  unsigned lo, hi, tag;
  __asm__ volatile("mfence\n\trdtscp\n\tlfence"
                   : "=a"(lo), "=d"(hi), "=c"(tag) : : "memory");
  *aux = tag;
  return ((uint64_t)hi << 32) | lo;
}
static inline uint64_t start_timer(void) { return read_timer(&tsc_aux_start); }
static inline uint64_t stop_timer(void) { return read_timer(&tsc_aux_stop); }
static inline double dtime(uint64_t start, uint64_t stop)
{
  /* Unsigned difference; no unverified constant overhead subtraction. */
  return (double)(stop - start);
}
#endif
