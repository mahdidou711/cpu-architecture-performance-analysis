/* Matrix benchmark harness: the kernels of tp1.c are included unchanged. */
#define TP1_NO_MAIN
#include "tp1.c"

__attribute__((noinline)) void transpose(void)
{
  for (int k = 0; k < N; ++k)
    for (int j = 0; j < N; ++j) YT[j][k] = XF[k][j];
}
__attribute__((noinline)) void transposed_product(void)
{
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j) {
      TYPE s = ZERO;
      for (int k = 0; k < N; ++k) s += AF[i][k] * YT[j][k];
      YF[i][j] = s;
    }
}
__attribute__((noinline)) void dot4(void)
{
  TYPE a = ZERO, b = ZERO, c = ZERO, d = ZERO;
  int i = 0;
  for (; i + 3 < N * N; i += 4) {
    a += BF[i] * CF[i]; b += BF[i+1] * CF[i+1];
    c += BF[i+2] * CF[i+2]; d += BF[i+3] * CF[i+3];
  }
  TYPE s = (a + b) + (c + d);
  for (; i < N * N; ++i) s += BF[i] * CF[i];
  SF = s;
}
void init_bench_inputs(void)
{
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j) {
      AF[i][j] = (TYPE)((3*i + 5*j + 1)%7 - 3);
      XF[i][j] = (TYPE)((2*i + 3*j + 2)%7 - 3);
      YF[i][j] = (TYPE)77;
      YT[i][j] = (TYPE)99;
      BF[i*N+j] = (TYPE)((i*N+j)%7-3);
      /* 64-bit intermediate: 3*(i*N+j) exceeds INT_MAX for N >= 26755. */
      CF[i*N+j] = (TYPE)((3LL*(i*N+j)+1)%7-3);
    }
}
/* Periodic oracle does no N^3 calculation and never reads YF/YT. */
int verify_bench(const char *op)
{
  if (!strcmp(op,"ps") || !strcmp(op,"ps4")) {
    long double s = 0;
    for (int i = 0; i < N*N; ++i) s += (long double)BF[i]*CF[i];
    return (long double)SF == s;
  }
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j) {
      if (!strcmp(op,"transpose")) {
        if (YT[j][i] != XF[i][j]) return 0;
      } else {
        long long cycle = 0, tail = 0;
        for (int k = 0; k < 7; ++k) {
          int term = ((3*i+5*k+1)%7-3)*((2*k+3*j+2)%7-3);
          cycle += term;
          if (k < N%7) tail += term;
        }
        if ((long double)YF[i][j] != (long double)(cycle*(N/7)+tail)) return 0;
      }
    }
  return 1;
}
#ifndef TP1_MATRIX_NO_MAIN
int main(int argc, char **argv)
{
  static const char *const ops[] = {"transpose", "trans_mul", "trans_total",
      "ps4", "mm_ijk", "mm_ikj", "mm_b_ijk", "ps"};
  if (argc != 2) {
    fprintf(stderr, "usage: %s transpose|trans_mul|trans_total|ps4|mm_ijk|mm_ikj|mm_b_ijk|ps\n", argv[0]);
    return 2;
  }
  const char *op = argv[1];
  int known = 0;
  for (unsigned i = 0; i < sizeof ops / sizeof ops[0]; ++i) known |= !strcmp(op, ops[i]);
  if (!known) { fprintf(stderr, "Unknown operation: %s\n", op); return 2; }
  if (!tsc_supported()) { fprintf(stderr, "RDTSCP/SSE2 unavailable\n"); return 2; }
  if (sizeof(TYPE) != 4) {
    fprintf(stderr, "matrix_extras requires a 4-byte TYPE (float, int); got " STR(TYPE) "\n");
    return 2;
  }
  init_bench_inputs();
  printf("# N=%d M=%d BL=%d TYPE=" STR(TYPE) " op=%s\n",N,M,BL,op);
  printf("# empty=");
  for (int i=0;i<101;++i) {
    uint64_t a=start_timer(), b=stop_timer();
    printf("%s%llu",i?";":"",(unsigned long long)(b-a));
  }
  printf("\n");
  if (!strcmp(op,"mm_ijk")) mm_ijk();
  else if (!strcmp(op,"mm_ikj")) mm_ikj();
  else if (!strcmp(op,"mm_b_ijk")) mm_b_ijk();
  else if (!strcmp(op,"ps")) ps();
  else {
    if (!strcmp(op,"trans_mul")) transpose();
    for (int m=0;m<M;++m) {
      uint64_t a=start_timer();
      if (!strcmp(op,"transpose")) transpose();
      else if (!strcmp(op,"trans_mul")) transposed_product();
      else if (!strcmp(op,"trans_total")) { transpose(); transposed_product(); }
      else if (!strcmp(op,"ps4")) dot4();
      else return 2;
      add_res(dtime(a,stop_timer()),m);
    }
    print_res(op, (!strcmp(op,"transpose") || !strcmp(op,"ps4")) ? (double)N*N : (double)N*N*N);
  }
  int ok=verify_bench(op);
  printf("# verify=%d\n",ok);
  if (!strcmp(op,"ps") || !strcmp(op,"ps4")) print_checksum(&SF,1);
  else if (!strcmp(op,"transpose")) print_matrix_checksum(YT);
  else print_matrix_checksum(YF);
  return ok ? 0 : 1;
}
#endif
