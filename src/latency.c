/* Explicit operand-specific x86 latency/throughput probes, no core-cycle claim. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tsc.h"
__attribute__((noinline)) static int int_add_1(long n)
{
  uint32_t x0=123456789u, y=0;
  __asm__ volatile("1: add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; add %[y], %[x0]; dec %[n]; jnz 1b"
    : [x0] "+&r"(x0), [n] "+&r"(n)
    : [y] "r"(y) : "cc");
  return x0==123456789u;
}
__attribute__((noinline)) static int int_add_4(long n)
{
  uint32_t x0=123456789u, x1=123456789u, x2=123456789u, x3=123456789u, y=0;
  __asm__ volatile("1: add %[y], %[x0]; add %[y], %[x1]; add %[y], %[x2]; add %[y], %[x3]; add %[y], %[x0]; add %[y], %[x1]; add %[y], %[x2]; add %[y], %[x3]; add %[y], %[x0]; add %[y], %[x1]; add %[y], %[x2]; add %[y], %[x3]; add %[y], %[x0]; add %[y], %[x1]; add %[y], %[x2]; add %[y], %[x3]; add %[y], %[x0]; add %[y], %[x1]; add %[y], %[x2]; add %[y], %[x3]; add %[y], %[x0]; add %[y], %[x1]; add %[y], %[x2]; add %[y], %[x3]; add %[y], %[x0]; add %[y], %[x1]; add %[y], %[x2]; add %[y], %[x3]; add %[y], %[x0]; add %[y], %[x1]; add %[y], %[x2]; add %[y], %[x3]; dec %[n]; jnz 1b"
    : [x0] "+&r"(x0), [x1] "+&r"(x1), [x2] "+&r"(x2), [x3] "+&r"(x3), [n] "+&r"(n)
    : [y] "r"(y) : "cc");
  return x0==123456789u && x1==123456789u && x2==123456789u && x3==123456789u;
}
__attribute__((noinline)) static int int_mul_1(long n)
{
  uint32_t x0=123456789u, y=1;
  __asm__ volatile("1: imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; imul %[y], %[x0]; dec %[n]; jnz 1b"
    : [x0] "+&r"(x0), [n] "+&r"(n)
    : [y] "r"(y) : "cc");
  return x0==123456789u;
}
__attribute__((noinline)) static int int_mul_4(long n)
{
  uint32_t x0=123456789u, x1=123456789u, x2=123456789u, x3=123456789u, y=1;
  __asm__ volatile("1: imul %[y], %[x0]; imul %[y], %[x1]; imul %[y], %[x2]; imul %[y], %[x3]; imul %[y], %[x0]; imul %[y], %[x1]; imul %[y], %[x2]; imul %[y], %[x3]; imul %[y], %[x0]; imul %[y], %[x1]; imul %[y], %[x2]; imul %[y], %[x3]; imul %[y], %[x0]; imul %[y], %[x1]; imul %[y], %[x2]; imul %[y], %[x3]; imul %[y], %[x0]; imul %[y], %[x1]; imul %[y], %[x2]; imul %[y], %[x3]; imul %[y], %[x0]; imul %[y], %[x1]; imul %[y], %[x2]; imul %[y], %[x3]; imul %[y], %[x0]; imul %[y], %[x1]; imul %[y], %[x2]; imul %[y], %[x3]; imul %[y], %[x0]; imul %[y], %[x1]; imul %[y], %[x2]; imul %[y], %[x3]; dec %[n]; jnz 1b"
    : [x0] "+&r"(x0), [x1] "+&r"(x1), [x2] "+&r"(x2), [x3] "+&r"(x3), [n] "+&r"(n)
    : [y] "r"(y) : "cc");
  return x0==123456789u && x1==123456789u && x2==123456789u && x3==123456789u;
}
__attribute__((noinline)) static int int_div_1(long n)
{
  uint32_t x0=123456789u, y=1;
  __asm__ volatile("1: mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; dec %[n]; jnz 1b"
    : [x0] "+&r"(x0), [n] "+&r"(n)
    : [y] "r"(y) : "cc", "eax", "edx");
  return x0==123456789u;
}
__attribute__((noinline)) static int int_div_4(long n)
{
  uint32_t x0=123456789u, x1=123456789u, x2=123456789u, x3=123456789u, y=1;
  __asm__ volatile("1: mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; div %[y]; mov %%eax, %[x3]; dec %[n]; jnz 1b"
    : [x0] "+&r"(x0), [x1] "+&r"(x1), [x2] "+&r"(x2), [x3] "+&r"(x3), [n] "+&r"(n)
    : [y] "r"(y) : "cc", "eax", "edx");
  return x0==123456789u && x1==123456789u && x2==123456789u && x3==123456789u;
}
__attribute__((noinline)) static int int_empty_1(long n)
{
  uint32_t x0=123456789u, y=1;
  __asm__ volatile("1: mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; dec %[n]; jnz 1b"
    : [x0] "+&r"(x0), [n] "+&r"(n)
    : [y] "r"(y) : "cc", "eax", "edx");
  return x0==123456789u;
}
__attribute__((noinline)) static int int_empty_4(long n)
{
  uint32_t x0=123456789u, x1=123456789u, x2=123456789u, x3=123456789u, y=1;
  __asm__ volatile("1: mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; mov %%eax, %[x3]; mov %[x0], %%eax; xor %%edx, %%edx; mov %%eax, %[x0]; mov %[x1], %%eax; xor %%edx, %%edx; mov %%eax, %[x1]; mov %[x2], %%eax; xor %%edx, %%edx; mov %%eax, %[x2]; mov %[x3], %%eax; xor %%edx, %%edx; mov %%eax, %[x3]; dec %[n]; jnz 1b"
    : [x0] "+&r"(x0), [x1] "+&r"(x1), [x2] "+&r"(x2), [x3] "+&r"(x3), [n] "+&r"(n)
    : [y] "r"(y) : "cc", "eax", "edx");
  return x0==123456789u && x1==123456789u && x2==123456789u && x3==123456789u;
}
__attribute__((noinline)) static int float_add_1(long n)
{
  float x0=1.0f, y=0;
  __asm__ volatile("1: addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; addss %[y], %[x0]; dec %[n]; jnz 1b"
    : [x0] "+&x"(x0), [n] "+&r"(n)
    : [y] "x"(y) : "cc");
  return x0==1.0f;
}
__attribute__((noinline)) static int float_add_4(long n)
{
  float x0=1.0f, x1=1.0f, x2=1.0f, x3=1.0f, y=0;
  __asm__ volatile("1: addss %[y], %[x0]; addss %[y], %[x1]; addss %[y], %[x2]; addss %[y], %[x3]; addss %[y], %[x0]; addss %[y], %[x1]; addss %[y], %[x2]; addss %[y], %[x3]; addss %[y], %[x0]; addss %[y], %[x1]; addss %[y], %[x2]; addss %[y], %[x3]; addss %[y], %[x0]; addss %[y], %[x1]; addss %[y], %[x2]; addss %[y], %[x3]; addss %[y], %[x0]; addss %[y], %[x1]; addss %[y], %[x2]; addss %[y], %[x3]; addss %[y], %[x0]; addss %[y], %[x1]; addss %[y], %[x2]; addss %[y], %[x3]; addss %[y], %[x0]; addss %[y], %[x1]; addss %[y], %[x2]; addss %[y], %[x3]; addss %[y], %[x0]; addss %[y], %[x1]; addss %[y], %[x2]; addss %[y], %[x3]; dec %[n]; jnz 1b"
    : [x0] "+&x"(x0), [x1] "+&x"(x1), [x2] "+&x"(x2), [x3] "+&x"(x3), [n] "+&r"(n)
    : [y] "x"(y) : "cc");
  return x0==1.0f && x1==1.0f && x2==1.0f && x3==1.0f;
}
__attribute__((noinline)) static int float_mul_1(long n)
{
  float x0=1.0f, y=1;
  __asm__ volatile("1: mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; mulss %[y], %[x0]; dec %[n]; jnz 1b"
    : [x0] "+&x"(x0), [n] "+&r"(n)
    : [y] "x"(y) : "cc");
  return x0==1.0f;
}
__attribute__((noinline)) static int float_mul_4(long n)
{
  float x0=1.0f, x1=1.0f, x2=1.0f, x3=1.0f, y=1;
  __asm__ volatile("1: mulss %[y], %[x0]; mulss %[y], %[x1]; mulss %[y], %[x2]; mulss %[y], %[x3]; mulss %[y], %[x0]; mulss %[y], %[x1]; mulss %[y], %[x2]; mulss %[y], %[x3]; mulss %[y], %[x0]; mulss %[y], %[x1]; mulss %[y], %[x2]; mulss %[y], %[x3]; mulss %[y], %[x0]; mulss %[y], %[x1]; mulss %[y], %[x2]; mulss %[y], %[x3]; mulss %[y], %[x0]; mulss %[y], %[x1]; mulss %[y], %[x2]; mulss %[y], %[x3]; mulss %[y], %[x0]; mulss %[y], %[x1]; mulss %[y], %[x2]; mulss %[y], %[x3]; mulss %[y], %[x0]; mulss %[y], %[x1]; mulss %[y], %[x2]; mulss %[y], %[x3]; mulss %[y], %[x0]; mulss %[y], %[x1]; mulss %[y], %[x2]; mulss %[y], %[x3]; dec %[n]; jnz 1b"
    : [x0] "+&x"(x0), [x1] "+&x"(x1), [x2] "+&x"(x2), [x3] "+&x"(x3), [n] "+&r"(n)
    : [y] "x"(y) : "cc");
  return x0==1.0f && x1==1.0f && x2==1.0f && x3==1.0f;
}
__attribute__((noinline)) static int float_div_1(long n)
{
  float x0=1.0f, y=1;
  __asm__ volatile("1: divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; divss %[y], %[x0]; dec %[n]; jnz 1b"
    : [x0] "+&x"(x0), [n] "+&r"(n)
    : [y] "x"(y) : "cc");
  return x0==1.0f;
}
__attribute__((noinline)) static int float_div_4(long n)
{
  float x0=1.0f, x1=1.0f, x2=1.0f, x3=1.0f, y=1;
  __asm__ volatile("1: divss %[y], %[x0]; divss %[y], %[x1]; divss %[y], %[x2]; divss %[y], %[x3]; divss %[y], %[x0]; divss %[y], %[x1]; divss %[y], %[x2]; divss %[y], %[x3]; divss %[y], %[x0]; divss %[y], %[x1]; divss %[y], %[x2]; divss %[y], %[x3]; divss %[y], %[x0]; divss %[y], %[x1]; divss %[y], %[x2]; divss %[y], %[x3]; divss %[y], %[x0]; divss %[y], %[x1]; divss %[y], %[x2]; divss %[y], %[x3]; divss %[y], %[x0]; divss %[y], %[x1]; divss %[y], %[x2]; divss %[y], %[x3]; divss %[y], %[x0]; divss %[y], %[x1]; divss %[y], %[x2]; divss %[y], %[x3]; divss %[y], %[x0]; divss %[y], %[x1]; divss %[y], %[x2]; divss %[y], %[x3]; dec %[n]; jnz 1b"
    : [x0] "+&x"(x0), [x1] "+&x"(x1), [x2] "+&x"(x2), [x3] "+&x"(x3), [n] "+&r"(n)
    : [y] "x"(y) : "cc");
  return x0==1.0f && x1==1.0f && x2==1.0f && x3==1.0f;
}
__attribute__((noinline)) static int float_empty_1(long n)
{
  float x0=1.0f, y=1;
  __asm__ volatile("1: dec %[n]; jnz 1b"
    : [x0] "+&x"(x0), [n] "+&r"(n)
    : [y] "x"(y) : "cc");
  return x0==1.0f;
}
__attribute__((noinline)) static int float_empty_4(long n)
{
  float x0=1.0f, x1=1.0f, x2=1.0f, x3=1.0f, y=1;
  __asm__ volatile("1: dec %[n]; jnz 1b"
    : [x0] "+&x"(x0), [x1] "+&x"(x1), [x2] "+&x"(x2), [x3] "+&x"(x3), [n] "+&r"(n)
    : [y] "x"(y) : "cc");
  return x0==1.0f && x1==1.0f && x2==1.0f && x3==1.0f;
}
int main(int argc,char **argv)
{
  if(argc!=3 || !tsc_supported()) return 2;
  long n=atol(argv[2]);
  if(n<=0 || n>1000000) return 2;
  int (*fn)(long)=NULL;
  if(!strcmp(argv[1],"int_add_1")) fn=int_add_1;
  if(!strcmp(argv[1],"int_add_4")) fn=int_add_4;
  if(!strcmp(argv[1],"int_mul_1")) fn=int_mul_1;
  if(!strcmp(argv[1],"int_mul_4")) fn=int_mul_4;
  if(!strcmp(argv[1],"int_div_1")) fn=int_div_1;
  if(!strcmp(argv[1],"int_div_4")) fn=int_div_4;
  if(!strcmp(argv[1],"int_empty_1")) fn=int_empty_1;
  if(!strcmp(argv[1],"int_empty_4")) fn=int_empty_4;
  if(!strcmp(argv[1],"float_add_1")) fn=float_add_1;
  if(!strcmp(argv[1],"float_add_4")) fn=float_add_4;
  if(!strcmp(argv[1],"float_mul_1")) fn=float_mul_1;
  if(!strcmp(argv[1],"float_mul_4")) fn=float_mul_4;
  if(!strcmp(argv[1],"float_div_1")) fn=float_div_1;
  if(!strcmp(argv[1],"float_div_4")) fn=float_div_4;
  if(!strcmp(argv[1],"float_empty_1")) fn=float_empty_1;
  if(!strcmp(argv[1],"float_empty_4")) fn=float_empty_4;
  if(!fn) return 2;
  unsigned tag;
  uint64_t warm=read_timer(&tag);
  while(read_timer(&tag)-warm<124800000u) if(!fn(2000)) return 3;
  printf("# empty=");
  for(int i=0;i<101;++i) {
    uint64_t a=start_timer(), b=stop_timer();
    printf("%s%llu",i?";":"",(unsigned long long)(b-a));
  }
  printf("\n");
  printf("sample,ticks,denominator,aux_start,aux_stop,valid\n");
  for(int i=0;i<31;++i) {
    uint64_t a=start_timer();
    int ok=fn(n);
    uint64_t b=stop_timer();
    printf("%d,%llu,%ld,%u,%u,%d\n",i,(unsigned long long)(b-a),32*n,
           tsc_aux_start,tsc_aux_stop,ok && tsc_aux_start==tsc_aux_stop);
    if(!ok) return 4;
  }
  return 0;
}
