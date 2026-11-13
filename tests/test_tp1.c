/* Include the real kernel functions; no duplicate implementation under test. */
#ifndef TP1_SOURCE
#define TP1_SOURCE "../src/tp1.c"
#endif
#define main tp1_demo_main
#define TP1_NO_MAIN
#include TP1_SOURCE
#undef main
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static int failures;
static const int floating_type = _Generic((TYPE)0, float: 1, double: 1, default: 0);
static long double expected[N][N], magnitude[N][N];
static TYPE saved_a[N][N], saved_x[N][N];

static void check(long double got, long double want, long double scale, size_t terms, const char *label)
{
  long double eps = _Generic((TYPE)0, float: FLT_EPSILON, double: DBL_EPSILON, default: 0.0L);
  long double tolerance = floating_type ? 8 * eps * terms * fmaxl(1, scale) : 0;
  if (!isfinite(got) || fabsl(got - want) > tolerance) {
    if (failures < 12) fprintf(stderr, "FAIL %s got=%.12Lg expected=%.12Lg tol=%.3Lg\n", label, got, want, tolerance);
    failures++;
  }
}

static void seed(int pattern)
{
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++) {
      int a = (i * 3 + j * 5 + 1) % 7;
      int x = (i * 5 + j * 2 + 3) % 7;
      AF[i][j] = floating_type ? (TYPE)((a - 3) / 7.0) : (TYPE)(a % 2);
      XF[i][j] = pattern == 1 ? (TYPE)(i == j) : pattern == 2 ? (TYPE)0 : floating_type ? (TYPE)((x - 3) / 9.0) : (TYPE)(x % 2);
      YF[i][j] = (TYPE)3; /* poison destination, including before repeated calls */
      BF[i * N + j] = floating_type ? (TYPE)((a - 3) / 7.0) : (TYPE)(a % 2);
      CF[i * N + j] = floating_type ? (TYPE)((x - 3) / 9.0) : (TYPE)(x % 2);
    }
}

static void matrix(void (*fn)(void), const char *label, int pattern)
{
  int before = failures;
  seed(pattern);
  memcpy(saved_a, AF, sizeof AF); memcpy(saved_x, XF, sizeof XF);
  for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) {
    expected[i][j] = 0; magnitude[i][j] = 0;
    for (int k = 0; k < N; k++) {
      long double product = (long double)AF[i][k] * XF[k][j];
      expected[i][j] += product; magnitude[i][j] += fabsl(product);
    }
  }
  for (int repeat = 0; repeat < 2; repeat++) {
    for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) YF[i][j] = (TYPE)3;
    fn();
    for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) check(YF[i][j], expected[i][j], magnitude[i][j], N + (N + BL - 1) / BL, label);
    if (memcmp(saved_a, AF, sizeof AF) || memcmp(saved_x, XF, sizeof XF)) { fprintf(stderr,"FAIL input modified\n"); failures++; }
  }
  fprintf(stderr, "%s %s pattern=%d failures=%d\n", failures == before ? "PASS" : "FAIL", label, pattern, failures - before);
}

int main(void)
{
#ifndef TP1_BASELINE
  if (!tsc_supported()) return EXIT_FAILURE;
#endif
  seed(0); zero();
  for (int i = 0; i < N * N; i++) check(BF[i], 0, 1, 1, "zero");
  void (*copies[])(void) = {copy_ij, copy_ji};
  void (*adds[])(void) = {add_ij, add_ji};
  for (int v = 0; v < 2; v++) {
    seed(0); copies[v]();
    for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) check(AF[i][j], YF[i][j], 1, 1, "copy");
    seed(0); memcpy(saved_a, AF, sizeof AF); adds[v]();
    for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) check(AF[i][j], (long double)saved_a[i][j] + M * (long double)YF[i][j], M * 3 + 1, M + 1, "add in-place M times");
  }
  seed(0); long double dot = 0, dot_abs = 0;
  for (int i = 0; i < N * N; i++) { long double product = (long double)BF[i] * CF[i]; dot += product; dot_abs += fabsl(product); }
  ps(); check(SF, dot, dot_abs, N * N, "dot");
  ps(); check(SF, dot, dot_abs, N * N, "dot repeated");
  for (int p = 0; p < 3; p++) {
    matrix(mm_ijk, "mm_ijk", p); matrix(mm_ikj, "mm_ikj", p); matrix(mm_b_ijk, "mm_b_ijk", p);
  }
  fprintf(stderr, "%s N=%d BL=%d M=%d TYPE=%s failures=%d\n", failures ? "FAIL" : "PASS", N, BL, M, STR(TYPE), failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
