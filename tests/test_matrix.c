#define BENCHMARK_MATRIX_NO_MAIN
#include "../src/matrix_extras.c"

static long double oracle[N][N];
static int check(void)
{
  for(int i=0;i<N;++i) for(int j=0;j<N;++j)
    if(fabsl((long double)YF[i][j]-oracle[i][j]) > 2e-5L*(1+fabsl(oracle[i][j]))) return 0;
  return 1;
}
int main(void)
{
  for(int pattern=0;pattern<3;++pattern) {
    init_bench_inputs();
    if(pattern) for(int i=0;i<N;++i) for(int j=0;j<N;++j) {
      /* Dyadic fractions plus non-dyadic values test numerical tolerance. */
      /* Integer TYPEs truncate fractions, so they use integer scales instead
         (a fractional scale would turn the data into all zeros). */
      double scale=((TYPE)0.5==0) ? (pattern==1?1.0:2.0) : (pattern==1?0.125:0.1);
      AF[i][j]=(TYPE)(((i*17+j*11+3)%19-9)*scale);
      XF[i][j]=(TYPE)(((i*7+j*13+5)%17-8)*scale);
      BF[i*N+j]=AF[i][j]; CF[i*N+j]=XF[i][j];
    }
    { /* Guard against degenerate (all-zero) test data. */
      int nzA=0,nzX=0;
      for(int i=0;i<N;++i) for(int j=0;j<N;++j) { nzA+=AF[i][j]!=0; nzX+=XF[i][j]!=0; }
      if(nzA==0||nzX==0) return 8;
    }
    for(int i=0;i<N;++i) for(int j=0;j<N;++j) {
      oracle[i][j]=0;
      for(int k=0;k<N;++k) oracle[i][j]+=(long double)AF[i][k]*XF[k][j];
    }
    for(int repeat=0;repeat<2;++repeat) {
      mm_ijk(); if(!check()) return 1;
      mm_ikj(); if(!check()) return 2;
      mm_b_ijk(); if(!check()) return 3;
      transpose();
      for(int i=0;i<N;++i) for(int j=0;j<N;++j) if(YT[j][i]!=XF[i][j]) return 4;
      transposed_product(); if(!check()) return 5;
      long double dot=0;
      for(int i=0;i<N*N;++i) dot+=(long double)BF[i]*CF[i];
      dot_product(); if(fabsl(SF-dot)>2e-5L*(1+fabsl(dot))) return 6;
      dot4(); if(fabsl(SF-dot)>2e-5L*(1+fabsl(dot))) return 7;
    }
  }
  fprintf(stderr, "PASS independent long-double matrix/dot oracles, repeated calls, transpose\n");
  return 0;
}
