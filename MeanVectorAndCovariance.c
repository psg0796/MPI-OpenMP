#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define INPUT_FILE "iris.data"

int getNumberOfVectorsInFile(char inputFileName[]) {
  FILE *fp;
  int size = 0;
  fp = fopen(inputFileName, "r");
  while(!feof(fp)) {
    char tmp[100];
    fscanf(fp, "%s\n", tmp);
    size++;
  }
  fclose(fp);

  return size;
}

void storeNumbersFromFileRead(char inputFileName[], float *numbers, int size) {
  FILE *fp;
  fp = fopen(inputFileName, "r");
  for(int i = 0; i < size; i++) {
    char tmp[40];
    float a, b, c, d;
    fscanf(fp, "%f,%f,%f,%f,%s\n", &a, &b, &c, &d, tmp);
    numbers[4*i + 0] = a;
    numbers[4*i + 1] = b;
    numbers[4*i + 2] = c;
    numbers[4*i + 3] = d;
  }
  fclose(fp);
}

void findSumVector(float SumVector[], float **VectorMatrix, int start, int end) {
  if(start == end) {
    for(int i = 0; i < 4; i++) {
      SumVector[i] = VectorMatrix[start][i];
    }
  } else if(start < end) {
    int mid = start + (end - start)/2;
    float SumVector1[4], SumVector2[4];
    findSumVector(SumVector1, VectorMatrix, start, mid);
    findSumVector(SumVector2, VectorMatrix, mid + 1, end);

    for(int i = 0; i < 4; i++) {
      SumVector[i] = SumVector1[i] + SumVector2[i];
    }
  }
}

void findMeanVector(float MeanVector[], float **VectorMatrix, int number_of_vectors) {
  float SumVector[4];
  findSumVector(SumVector, VectorMatrix, 0, number_of_vectors - 1);

  for(int i = 0; i < 4; i++) {
    MeanVector[i] = SumVector[i] / number_of_vectors;
  }
}

/**
 * Function calculates the deviation matrix from mean; i.e, Xij - Xj(mean)
 */
void findDeviationMatrix(float **DeviationMatrix, float MeanVector[], float **VectorMatrix, int start, int end) {
  if(end - start < 4) {
    for(int i = start; i <= end; i++) {
      for(int j = 0; j < 4; j++) {
        DeviationMatrix[i][j] = VectorMatrix[i][j] - MeanVector[j];
      }
    }
  } else if(start < end) {
    int mid = start + (end - start)/2;
    findDeviationMatrix(DeviationMatrix, MeanVector, VectorMatrix, start, mid);
    findDeviationMatrix(DeviationMatrix, MeanVector, VectorMatrix, mid + 1, end);
  }
}

/**
 * Dot Product of two sub-vectors having elements from start to end in respective vectors A & B
 */
float findDotProduct(float A[], float B[], int start, int end) {
  if(end - start < 4) {
    float sum = 0;
    for(int i = start; i <= end; i++) {
      sum += (A[i] * B[i]);
    }
    return sum;
  } else {
    int mid = start + (end - start)/2;
    float sum1 = findDotProduct(A, B, start, mid);
    float sum2 = findDotProduct(A, B, mid + 1, end);
    return sum1 + sum2;
  }
}

void findCovarianceMatrix(float **CovarianceMatrix, float MeanVector[], float **VectorMatrix, int number_of_vectors) {
  float **DeviationMatrix, **DeviationMatrixTranspose;
  DeviationMatrix = (float**)malloc(number_of_vectors*sizeof(float*));
  DeviationMatrixTranspose = (float**)malloc(4*sizeof(float*));
  for(int i = 0; i < number_of_vectors; i++) {
    DeviationMatrix[i] = (float*)malloc(4*sizeof(float));
  }

  for(int i = 0; i < 4; i++) {
    DeviationMatrixTranspose[i] = (float*)malloc(number_of_vectors*sizeof(float));
  }

  findDeviationMatrix(DeviationMatrix, MeanVector, VectorMatrix, 0, number_of_vectors - 1);

  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < number_of_vectors; j++) {
      DeviationMatrixTranspose[i][j] = DeviationMatrix[j][i];
      // printf("%f ", DeviationMatrixTranspose[i][j]);
    }
    // printf("\n");
  }

  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      CovarianceMatrix[i][j] = findDotProduct(DeviationMatrixTranspose[i], DeviationMatrixTranspose[j], 0, number_of_vectors - 1);
    }
  }
}

int main(int argc, char **argv) {
  clock_t start, end;
  double cpu_time_used;
  int number_of_vectors = 0, number_of_vectors_per_process = 0;
  float **VectorMatrix, CovarianceMatrix[4][4], *VectorMatrixLinear, VectorMatrixLinearBuff[1000000];
  float MeanVector[4], SumVector[4];
  int my_rank, num_process, root = 0;

  MPI_Init(&argc, &argv); // Initialise MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // Get my rank
  MPI_Comm_size(MPI_COMM_WORLD, &num_process); // Get the total number of processors

  if(my_rank == root) {
    number_of_vectors = getNumberOfVectorsInFile(INPUT_FILE);

    if(number_of_vectors % num_process != 0) {
      printf("Please enter number of processes such that it is a factor of \t%d\n Sorry for inconvenience.\n", number_of_vectors);
      exit(0);
    }

    VectorMatrixLinear = (float*)malloc(4*number_of_vectors*sizeof(float));

    storeNumbersFromFileRead(INPUT_FILE, VectorMatrixLinear, number_of_vectors);
  }

  MPI_Bcast(&number_of_vectors, 1, MPI_INT, root, MPI_COMM_WORLD);

	MPI_Scatter(VectorMatrixLinear, 4 * (number_of_vectors/num_process), MPI_FLOAT, VectorMatrixLinearBuff, 4 * (number_of_vectors/num_process), MPI_FLOAT, root, MPI_COMM_WORLD);

  number_of_vectors_per_process = number_of_vectors / num_process;
  VectorMatrix = (float**)malloc(number_of_vectors_per_process*sizeof(float*));
  for(int i = 0; i < number_of_vectors_per_process; i++) {
    VectorMatrix[i] = (float*)malloc(4*sizeof(float));
    VectorMatrix[i][0] = VectorMatrixLinearBuff[4*i + 0];
    VectorMatrix[i][1] = VectorMatrixLinearBuff[4*i + 1];
    VectorMatrix[i][2] = VectorMatrixLinearBuff[4*i + 2];
    VectorMatrix[i][3] = VectorMatrixLinearBuff[4*i + 3];
  }

  float SumVectorLocal[4];
 
  start = clock();
  findSumVector(SumVectorLocal, VectorMatrix, 0, number_of_vectors_per_process - 1);

  for(int i = 0; i < 4; i++) {
    MPI_Allreduce(&SumVectorLocal[i], &SumVector[i], 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
  }

  for(int i = 0; i < 4; i++) {
    MeanVector[i] = SumVector[i] / number_of_vectors;
  }

  if(my_rank == root) {
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("CPU time for Mean Vector = %lf\n", cpu_time_used);

    printf("Mean Vector:\n\n");
    for(int i = 0; i < 4; i++) {
      printf("%f\t", MeanVector[i]);
    }
    printf("\n\n");
    start = clock();
  }

  float **DeviationMatrix, **DeviationMatrixTranspose;
  DeviationMatrix = (float**)malloc(number_of_vectors_per_process*sizeof(float*));
  DeviationMatrixTranspose = (float**)malloc(4*sizeof(float*));
  for(int i = 0; i < number_of_vectors_per_process; i++) {
    DeviationMatrix[i] = (float*)malloc(4*sizeof(float));
  }

  for(int i = 0; i < 4; i++) {
    DeviationMatrixTranspose[i] = (float*)malloc(number_of_vectors_per_process*sizeof(float));
  }

  findDeviationMatrix(DeviationMatrix, MeanVector, VectorMatrix, 0, number_of_vectors_per_process - 1);

  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < number_of_vectors_per_process; j++) {
      DeviationMatrixTranspose[i][j] = DeviationMatrix[j][i];
    }
  }

  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      float tmp = findDotProduct(DeviationMatrixTranspose[i], DeviationMatrixTranspose[j], 0, number_of_vectors_per_process - 1);
      MPI_Reduce(&tmp, &CovarianceMatrix[i][j], 1, MPI_FLOAT, MPI_SUM, root, MPI_COMM_WORLD);
    }
  }

  if(my_rank == root) {
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("CPU time for Covariance Matrix = %lf\n", cpu_time_used);

    printf("Covariance Matrix:\n\n");
    for(int i = 0; i < 4; i++) {
      for(int j = 0; j < 4; j++) {
        printf("%f\t", CovarianceMatrix[i][j]);
      }
      printf("\n");
    }
    printf("\n");
  }
  MPI_Finalize();
  return 0;
}