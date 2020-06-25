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

void storeNumbersFromFileRead(char inputFileName[], float (*numbers)[4], long size) {
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

float getEuclideanDistance(float A[], float B[], long size) {
  float dist = 0;
  for(long i = 0; i < size; i++) {
    dist += ((A[i] - B[i])*(A[i] - B[i]));
  }
  return dist;
}

long getNearestCluster(float *VectorMatrix, float (*KMeanVectors)[4], long K) {
  long minK = 0, k, i;
  float distance[K];

  for(k = 0; k < K; k++) {
    distance[k] = 0;
  }

  for(i = 0; i < 4; i++)
    for(k = 0; k < K; k++) {
      distance[k] += ((VectorMatrix[i] - KMeanVectors[k][i])*(VectorMatrix[i] - KMeanVectors[k][i]));
    }

  for(int k = 1; k < K; k++) {
    if(distance[k] < distance[minK]) {
      minK = k;
    }
  }

  return minK;
}

int updateClusterCenters(long start, long end, float (*VectorMatrix)[4], float (*KMeanVectors)[4], long K, long ItemCountPerCluster[], long VectorClusterCenter[]) {
  int reachedConvergence = 1;
  long i;

  for(i = start; i <= end; i++) {
    long minK = getNearestCluster(VectorMatrix[i], KMeanVectors, K);

    ItemCountPerCluster[minK]++;

    if(VectorClusterCenter[i] != minK) {
      reachedConvergence = 0;
      VectorClusterCenter[i] = minK;
    }
  }

  return reachedConvergence;
}

int main() {
  omp_set_num_threads(MAX_NUM_THREADS);
  long number_of_vectors = 0;
  number_of_vectors = getNumberOfVectorsInFile(INPUT_FILE);
  clock_t start, end;
  double cpu_time_used;

  float VectorMatrix[number_of_vectors][4];

  storeNumbersFromFileRead(INPUT_FILE, VectorMatrix, number_of_vectors);

  long K;
  printf("Enter the value of K: ");
  scanf("%ld", &K);

  float KMeanVectors[K][4];
  long ItemCountPerCluster[K], VectorClusterCenter[number_of_vectors];
  /**
   * Initialization of K clusters
   */
  for(long i = 0; i < K; i++) {
    for(int j = 0; j < 4; j++) {
      KMeanVectors[i][j] = VectorMatrix[i][j];
    }
  }
  for(long i = 0; i < number_of_vectors; i++) {
    VectorClusterCenter[i] = rand()%K;
  }

  int reachedConvergence = 0;
  start = clock();
  long i, j;
  while(!reachedConvergence) {
    reachedConvergence = 1;
    /**
     * Initialize the number of data points in each cluster
     */
    for(i = 0; i < K; i++) {
      ItemCountPerCluster[i] = 0;
    }

    /**
     * Find nearest cluster for each data item.
     * Returns 0 if the convergence is not reached else 1.
     */
    long w_s = number_of_vectors/MAX_NUM_THREADS;
    #pragma omp parallel for reduction(*:reachedConvergence) reduction(+:ItemCountPerCluster)
    for(i = 0; i < MAX_NUM_THREADS; i++) {
      long start = i*w_s, end = start + w_s - 1;
      reachedConvergence *= updateClusterCenters(start, end, VectorMatrix, KMeanVectors, K, ItemCountPerCluster, VectorClusterCenter);
    }
    /**
     * Initialize the K mean vectors.
     */
    for(i = 0; i < K; i++) {
      if(ItemCountPerCluster[i] != 0) {
        for(j = 0; j < 4; j++) {
          KMeanVectors[i][j] = 0;
        }
      }
    }

    /**
     * Update each mean vector with the summation of all the vectors falling in that cluster.
     * After taking the summation, the mean of clusters are calculated in the followed loop.
     */
    #pragma omp parallel for private(i) reduction(+:KMeanVectors)
      for(j = 0; j < 4; j++) {
        for(i = 0; i < number_of_vectors; i++) {
          KMeanVectors[VectorClusterCenter[i]][j] += VectorMatrix[i][j];
      }
    }

    for(i = 0; i < K; i++) {
      if(ItemCountPerCluster[i] != 0) {
        for(j = 0; j < 4; j++) {
          KMeanVectors[i][j] /= ItemCountPerCluster[i];
        }
      }
    }
  }

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("CPU time = %lf\n", cpu_time_used);

  for(long i = 0; i < K; i++) {
    for(long j = 0; j < 4; j++) {
      printf("%f\t", KMeanVectors[i][j]);
    }
    printf("\n");
  }
  printf("\n");

  return 0;
}
