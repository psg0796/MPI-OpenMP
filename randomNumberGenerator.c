#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
  int N, num_process, printCount, my_rank;
  time_t t;
  double elapsed_time;

  MPI_Init(&argc, &argv); // Initialise MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // Get my rank
  MPI_Comm_size(MPI_COMM_WORLD, &num_process); // Get the total number of processors
  FILE *fp;


  if(my_rank == 0) {
    printf("Enter the number of random numbers to be generated \n");
    scanf("%d", &N);
    printCount = N/num_process;
    elapsed_time = - MPI_Wtime();
    fp = fopen("randomNumbers.txt", "w");
    for(int i = 0; i < N%num_process; i++) {
      fprintf(fp, "%d\n", rand());
    }
    fclose(fp);
  }

  MPI_Bcast(&printCount, 1, MPI_INT, 0, MPI_COMM_WORLD);

  srand((unsigned) time(&t));

  fp = fopen("randomNumbers.txt", "a");

  for (long i = 0; i < printCount; i++) {
    fprintf(fp, "%d\n", rand());
  }

  fclose(fp);

  if(my_rank == 0){  
    elapsed_time += MPI_Wtime();
    printf("Time taken = %f\n\n", elapsed_time);
  }

  MPI_Finalize();

  return 0;
}