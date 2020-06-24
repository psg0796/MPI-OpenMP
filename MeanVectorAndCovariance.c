#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define MAX_NUM_THREADS 2
#define INPUT_FILE "iris.data"

long getNumberOfVectorsInFile(char inputFileName[]) {
  FILE *fp;
  long size = 0;
  fp = fopen(inputFileName, "r");
  while(!feof(fp)) {
    char tmp[100];
    fscanf(fp, "%s\n", tmp);
    size++;
  }
  fclose(fp);

  return size;
}

void storeNumbersFromFileRead(char inputFileName[], float **numbers, long size) {
  FILE *fp;
  fp = fopen(inputFileName, "r");
  for(int i = 0; i < size; i++) {
    char tmp[40];
    float a, b, c, d;
    fscanf(fp, "%f,%f,%f,%f,%s\n", &a, &b, &c, &d, tmp);
    numbers[i][0] = a;
    numbers[i][1] = b;
    numbers[i][2] = c;
    numbers[i][3] = d;
  }
  fclose(fp);
}

void findSumVector(float SumVector[], float **VectorMatrix, long start, long end) {
  if(start == end) {
    for(int i = 0; i < 4; i++) {
      SumVector[i] = VectorMatrix[start][i];
    }
  } else if(start < end) {
    long mid = start + (end - start)/2;
    float SumVector1[4], SumVector2[4];
    findSumVector(SumVector1, VectorMatrix, start, mid);
    findSumVector(SumVector2, VectorMatrix, mid + 1, end);

    for(int i = 0; i < 4; i++) {
      SumVector[i] = SumVector1[i] + SumVector2[i];
    }
  }
}

void findMeanVector(float MeanVector[], float **VectorMatrix, long number_of_vectors) {
  float SumVector[4] = {0};
  int i, j;
  // findSumVector(SumVector, VectorMatrix, 0, number_of_vectors - 1);

  #pragma omp parallel for reduction(+:SumVector) private(j)
    for(i = 0; i < number_of_vectors; i++) {
      for(j = 0; j < 4; j++) {
        SumVector[j] += VectorMatrix[i][j];
      } 
    }

  #pragma omp parallel for
  for(i = 0; i < 4; i++) {
    MeanVector[i] = SumVector[i] / number_of_vectors;
  }
}

/**
 * Function calculates the deviation matrix from mean; i.e, Xij - Xj(mean)
 */
void findDeviationMatrix(float **DeviationMatrix, float MeanVector[], float **VectorMatrix, long start, long end) {
  if(end - start < 4) {
    for(long i = start; i <= end; i++) {
      for(long j = 0; j < 4; j++) {
        DeviationMatrix[i][j] = VectorMatrix[i][j] - MeanVector[j];
      }
    }
  } else if(start < end) {
    long mid = start + (end - start)/2;
    findDeviationMatrix(DeviationMatrix, MeanVector, VectorMatrix, start, mid);
    findDeviationMatrix(DeviationMatrix, MeanVector, VectorMatrix, mid + 1, end);
  }
}

/**
 * Dot Product of two sub-vectors having elements from start to end in respective vectors A & B
 */
float findDotProduct(float A[], float B[], long start, long end) {
  if(end - start < 4) {
    float sum = 0;
    for(long i = start; i <= end; i++) {
      sum += (A[i] * B[i]);
    }
    return sum;
  } else {
    long mid = start + (end - start)/2;
    float sum1 = findDotProduct(A, B, start, mid);
    float sum2 = findDotProduct(A, B, mid + 1, end);
    return sum1 + sum2;
  }
}

void findCovarianceMatrix(float **CovarianceMatrix, float MeanVector[], float **VectorMatrix, long number_of_vectors) {
  float **DeviationMatrix, **DeviationMatrixTranspose;
  DeviationMatrix = (float**)malloc(number_of_vectors*sizeof(float*));
  DeviationMatrixTranspose = (float**)malloc(4*sizeof(float*));
  for(long i = 0; i < number_of_vectors; i++) {
    DeviationMatrix[i] = (float*)malloc(4*sizeof(float));
  }

  for(int i = 0; i < 4; i++) {
    DeviationMatrixTranspose[i] = (float*)malloc(number_of_vectors*sizeof(float));
  }

  findDeviationMatrix(DeviationMatrix, MeanVector, VectorMatrix, 0, number_of_vectors - 1);

  for(long i = 0; i < 4; i++) {
    for(long j = 0; j < number_of_vectors; j++) {
      DeviationMatrixTranspose[i][j] = DeviationMatrix[j][i];
      // printf("%f ", DeviationMatrixTranspose[i][j]);
    }
    // printf("\n");
  }

  for(long i = 0; i < 4; i++) {
    for(long j = 0; j < 4; j++) {
      CovarianceMatrix[i][j] = findDotProduct(DeviationMatrixTranspose[i], DeviationMatrixTranspose[j], 0, number_of_vectors - 1);
    }
  }
}

int main() {
  omp_set_num_threads(MAX_NUM_THREADS);
  clock_t start, end;
  double cpu_time_used;
  long number_of_vectors = 0;
  number_of_vectors = getNumberOfVectorsInFile(INPUT_FILE);

  float **VectorMatrix, **CovarianceMatrix;
  float MeanVector[4];
  VectorMatrix = (float**)malloc(number_of_vectors*sizeof(float*));
  CovarianceMatrix = (float**)malloc(4*sizeof(float*));

  for(long i = 0; i < number_of_vectors; i++) {
    VectorMatrix[i] = (float*)malloc(4*sizeof(float));
  }

  for(long i = 0; i < 4; i++) {
    CovarianceMatrix[i] = (float*)malloc(4*sizeof(float));
  }

  storeNumbersFromFileRead(INPUT_FILE, VectorMatrix, number_of_vectors);

  start = clock();
  findMeanVector(MeanVector, VectorMatrix, number_of_vectors);

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("CPU time for Mean Vector = %lf\n", cpu_time_used);

  printf("Mean Vector:\n\n");
  for(int i = 0; i < 4; i++) {
    printf("%f\t", MeanVector[i]);
  }
  printf("\n\n");

  start = clock();

  findCovarianceMatrix(CovarianceMatrix, MeanVector, VectorMatrix, number_of_vectors);

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

  return 0;
}