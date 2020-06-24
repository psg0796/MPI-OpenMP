#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define MAX_NUM_THREADS 4

int main() {
  omp_set_num_threads(MAX_NUM_THREADS);
  long N;
  FILE *fp;
  time_t t;
  clock_t start, end;
  double cpu_time_used;

  printf("Enter the number of random numbers to be generated \n");
  scanf("%ld", &N);

  start = clock();
  fp = fopen("randomNumbers.txt", "w");
  srand((unsigned) time(&t));
  long i;
  #pragma omp parallel for
  for (i = 0; i < N; i++) {
    fprintf(fp, "%d\n", rand());
  }
  fclose(fp);

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("CPU time = %lf\n", cpu_time_used);

  return 0;
}