#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
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
  for (long i = 0; i < N; i++) {
    fprintf(fp, "%d\n", rand());
  }
  fclose(fp);

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("CPU time = %lf\n", cpu_time_used);

  return 0;
}