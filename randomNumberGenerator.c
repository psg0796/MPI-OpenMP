#include <stdio.h>
#include <stdlib.h>

int main() {
  long N;
  FILE *fp;
  time_t t;

  printf("Enter the number of random numbers to be generated \n");
  scanf("%ld", &N);

  fp = fopen("randomNumbers.txt", "w");
  srand((unsigned) time(&t));
  for (long i = 0; i < N; i++) {
    fprintf(fp, "%d\n", rand());
  }
  fclose(fp);

  return 0;
}