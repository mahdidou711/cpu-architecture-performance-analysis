#include <stdio.h>
#include <stdint.h>

#ifndef N
#define N 1000 // Matrix dimension and square root of the vector length.
#endif
#ifndef M
#define M 16 // Number of samples per kernel.
#endif
#ifndef TYPE
#define TYPE float // Element type (for example, float, double, or int).
#endif

/* A C cast works for all supported TYPEs; the preprocessor cannot compare types. */
#define ZERO ((TYPE)0)

#ifndef BL
#define BL 16 // Tile dimension for blocked matrix multiplication.
#endif

#define STR1(x) #x
#define STR(x) STR1(x)

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/* int loop indices and products N*N must remain representable. */
#if N <= 0 || N > 46340 || M <= 0 || M > INT_MAX || BL <= 0 || BL > INT_MAX - N
#error "Require 1 <= N <= 46340, M > 0, 1 <= BL <= INT_MAX-N"
#endif

/* TSC ticks are not core clock cycles; see docs/METHODOLOGY.md. */
#include "tsc.h"

TYPE AF[N][N], YF[N][N], XF[N][N], YT[N][N], // N by N matrices
    BF[N * N], CF[N * N],                    // Vectors of length N squared
    SF;                                      // Accumulator

unsigned aux_begin[M], aux_end[M];
int valid_sample[M];
double samples[M]; // Elapsed TSC ticks for each sample.

static inline int min(int a, int b)
{
  if (a < b)
    return a;
  else
    return b;
}

uint64_t t_start;
double t_elapsed;

// Record the duration and migration status for one sample.
void add_res(double res, int where)
{
  samples[where] = res;
  aux_begin[where] = tsc_aux_start;
  aux_end[where] = tsc_aux_stop;
  valid_sample[where] = tsc_aux_start == tsc_aux_stop && isfinite(res) && res >= 0;
}

// Write sample CSV and normalize ticks by the supplied iteration count.
void print_res(const char *funcname, double nb_iter)
{
  /* Raw samples are always retained, including invalid/migrated ones. */
  printf("function,sample,ticks,denominator,ticks_per_iteration,aux_start,aux_stop,valid\n");
  for (int i = 0; i < M; i++)
    printf("%s,%d,%.0f,%.0f,%.12g,%u,%u,%d\n", funcname, i,
           samples[i], nb_iter, samples[i] / nb_iter,
           aux_begin[i], aux_end[i], valid_sample[i]);
}

// Separate sample groups in the output.
void separator(void)
{
  printf("\n");
}

// Vector zeroing
void zero(void)
{

  int i, m;

  for (m = 0; m < M; m++)
  {
    t_start = start_timer();

    for (i = 0; i < (N * N); i++)
      BF[i] = ZERO;
    t_elapsed = dtime(t_start, stop_timer());
    add_res(t_elapsed, m);
  }
  print_res("ZERO", (N * N));
  separator();
}

// Matrix copy
void copy_ij(void)
{
  int i, j, m;

  for (m = 0; m < M; m++)
  {
    t_start = start_timer();

    for (i = 0; i < N; i++)
      for (j = 0; j < N; j++)
        AF[i][j] = YF[i][j];
    t_elapsed = dtime(t_start, stop_timer());
    add_res(t_elapsed, m);
  }
  print_res("COPY_ij", (N * N));
  separator();
}

void copy_ji(void)
{
  int i, j, m;

  for (m = 0; m < M; m++)
  {
    t_start = start_timer();

    for (j = 0; j < N; j++)
      for (i = 0; i < N; i++)
        AF[i][j] = YF[i][j];
    t_elapsed = dtime(t_start, stop_timer());
    add_res(t_elapsed, m);
  }
  print_res("COPY_ji", (double)(N * N));
  separator();
}

// Matrix addition
void add_ij(void)
{
  int i, j, m;

  for (m = 0; m < M; m++)
  {
    t_start = start_timer();

    for (i = 0; i < N; i++)
      for (j = 0; j < N; j++)
        AF[i][j] += YF[i][j];
    t_elapsed = dtime(t_start, stop_timer());
    add_res(t_elapsed, m);
  }
  print_res("ADD_ij", (double)(N * N));
  separator();
}

void add_ji(void)
{
  int i, j, m;

  for (m = 0; m < M; m++)
  {
    t_start = start_timer();

    for (j = 0; j < N; j++)
      for (i = 0; i < N; i++)
        AF[i][j] += YF[i][j];
    t_elapsed = dtime(t_start, stop_timer());
    add_res(t_elapsed, m);
  }
  print_res("ADD_ji", (double)(N * N));
  separator();
}

// Dot product
void dot_product(void)
{
  int i, m;

  for (m = 0; m < M; m++)
  {
    t_start = start_timer();

    SF = ZERO;
    for (i = 0; i < (N * N); i++)
    {
      SF += BF[i] * CF[i];
    }
    t_elapsed = dtime(t_start, stop_timer());
    add_res(t_elapsed, m);
  }
  print_res("PS", (double)(N * N));
  separator();
}

// Matrix multiplication in ijk order
void mm_ijk(void)
{
  int i, j, k, m;

  for (m = 0; m < M; m++)
  {
    t_start = start_timer();

    for (i = 0; i < N; i++)
      for (j = 0; j < N; j++)
      {
        SF = ZERO;
        for (k = 0; k < N; k++)
          SF += AF[i][k] * XF[k][j];
        YF[i][j] = SF;
      }
    t_elapsed = dtime(t_start, stop_timer());
    add_res(t_elapsed, m);
  }
  print_res("MM_ijk", ((double)N * N * N));
  separator();
}

// Matrix multiplication in ikj order
void mm_ikj(void)
{
  int i, j, k, m;
  for (m = 0; m < M; m++)
  {
    t_start = start_timer();
    /* Each sample computes YF = AF * XF, independent of old output. */
    for (i = 0; i < N; i++)
      for (j = 0; j < N; j++)
        YF[i][j] = ZERO;

    for (i = 0; i < N; i++)
      for (k = 0; k < N; k++)
      {
        SF = AF[i][k];
        for (j = 0; j < N; j++)
          YF[i][j] += SF * XF[k][j];
      }
    t_elapsed = dtime(t_start, stop_timer());
    add_res(t_elapsed, m);
  }
  print_res("MM_ikj", ((double)N * N * N));
  separator();
}

// Blocked matrix multiplication
void mm_b_ijk(void)
{
  int i, j, k, m, jj, kk;

  for (m = 0; m < M; m++)
  {
    t_start = start_timer();
    /* Each sample computes YF = AF * XF, independent of old output. */
    for (i = 0; i < N; i++)
      for (j = 0; j < N; j++)
        YF[i][j] = ZERO;


    for (jj = 0; jj < N; jj += BL)
      for (kk = 0; kk < N; kk += BL)
        for (i = 0; i < N; i++)
        {
          for (j = jj; j < min(jj + BL, N); j++)
          {
            SF = ZERO;
            for (k = kk; k < min(kk + BL, N); k++)
              SF += AF[i][k] * XF[k][j];
            YF[i][j] += SF;
          }
        }

    t_elapsed = dtime(t_start, stop_timer());
    add_res(t_elapsed, m);
  }
  print_res("MM_B_ijk", ((double)N * N * N));

  separator();
}

/* Deterministic, bounded inputs. Call before each independent procedure.
   Integer matrix sums <= N, dot <= N*N; additions require M+1 <= TYPE_MAX.
   The small nonnegative integer data are also exactly representable as float. */
void init_inputs(void)
{
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++) {
      AF[i][j] = (TYPE)((i + j) % 2);
      XF[i][j] = (TYPE)((i + 2 * j + 1) % 2);
      YF[i][j] = (TYPE)((i + j + 1) % 2);
      BF[i * N + j] = (TYPE)((i + j) % 2);
      CF[i * N + j] = (TYPE)((i + 2 * j + 1) % 2);
    }
}

/* Visible use outside the interval, without volatile in the arithmetic loops. */
void print_checksum(const TYPE *data, size_t count)
{
  long double sum = 0;
  for (size_t i = 0; i < count; i++) sum += data[i];
  printf("# checksum=%.18Lg\n", sum);
}

void print_matrix_checksum(TYPE data[N][N])
{
  long double sum = 0;
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++) sum += data[i][j];
  printf("# checksum=%.18Lg\n", sum);
}

#ifndef BENCHMARK_NO_MAIN
int main(int argc, char **argv)
{
  if (!tsc_supported()) {
    fprintf(stderr, "RDTSCP/SSE2 unavailable\n");
    return EXIT_FAILURE;
  }
  const char *op = argc > 1 ? argv[1] : "mm_ijk";
  long double type_max = _Generic((TYPE)0, char: CHAR_MAX, short: SHRT_MAX,
      int: INT_MAX, uint64_t: UINT64_MAX, float: FLT_MAX, double: DBL_MAX);
  long double bound = !strcmp(op, "ps") ? (long double)N * N :
      !strncmp(op, "mm_", 3) ? N : !strncmp(op, "add_", 4) ? (long double)M + 1 : 1;
  if (bound > type_max) {
    fprintf(stderr, "Input bound exceeds TYPE range for this operation\n");
    return EXIT_FAILURE;
  }
  static const char *const ops[] = {"zero", "ps", "copy_ij", "copy_ji", "add_ij",
      "add_ji", "mm_ijk", "mm_ikj", "mm_b_ijk"};
  int known = 0;
  for (unsigned i = 0; i < sizeof ops / sizeof ops[0]; i++) known |= !strcmp(op, ops[i]);
  if (!known) { /* reject before printing any CSV header */
    fprintf(stderr, "Unknown operation: %s\n", op);
    return EXIT_FAILURE;
  }
  init_inputs();
  printf("# N=%d M=%d BL=%d TYPE=" STR(TYPE) " unit=TSC_ticks op=%s\n", N, M, BL, op);
  if (!strcmp(op, "zero")) { zero(); print_checksum(BF, N * N); }
  else if (!strcmp(op, "ps")) { dot_product(); print_checksum(&SF, 1); }
  else if (!strcmp(op, "copy_ij")) { copy_ij(); print_matrix_checksum(AF); }
  else if (!strcmp(op, "copy_ji")) { copy_ji(); print_matrix_checksum(AF); }
  else if (!strcmp(op, "add_ij")) { add_ij(); print_matrix_checksum(AF); }
  else if (!strcmp(op, "add_ji")) { add_ji(); print_matrix_checksum(AF); }
  else if (!strcmp(op, "mm_ijk")) { mm_ijk(); print_matrix_checksum(YF); }
  else if (!strcmp(op, "mm_ikj")) { mm_ikj(); print_matrix_checksum(YF); }
  else if (!strcmp(op, "mm_b_ijk")) { mm_b_ijk(); print_matrix_checksum(YF); }
  else { fprintf(stderr, "Unknown operation: %s\n", op); return EXIT_FAILURE; }
  return EXIT_SUCCESS;
}
#endif
