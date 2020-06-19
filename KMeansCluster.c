#include <stdio.h>
#include <stdlib.h>

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

float getEuclideanDistance(float A[], float B[], long size) {
  float dist = 0;
  for(long i = 0; i < size; i++) {
    dist += ((A[i] - B[i])*(A[i] - B[i]));
  }
  return dist;
}

long getNearestCluster(float *VectorMatrix, float **KMeanVectors, long K) {
  long minK = 0;
  float minDistance = getEuclideanDistance(VectorMatrix, KMeanVectors[0], 4);
  for(long k = 1; k < K; k++) {
    float distance = getEuclideanDistance(VectorMatrix, KMeanVectors[k], 4);
    if(distance < minDistance) {
      minDistance = distance;
      minK = k;
    }
  }
  return minK;
}

int updateClusterCenters(long start, long end, float **VectorMatrix, float **KMeanVectors, long K, long ItemCountPerCluster[], long VectorClusterCenter[]) {
  int reachedConvergence = 1;

  for(long i = start; i <= end; i++) {
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
  long number_of_vectors = 0;
  number_of_vectors = getNumberOfVectorsInFile(INPUT_FILE);

  float **VectorMatrix;
  VectorMatrix = (float**)malloc(number_of_vectors*sizeof(float*));
  for(long i = 0; i < number_of_vectors; i++) {
    VectorMatrix[i] = (float*)malloc(4*sizeof(float));
  }
  
  storeNumbersFromFileRead(INPUT_FILE, VectorMatrix, number_of_vectors);

  long K;
  printf("Enter the value of K: ");
  scanf("%ld", &K);

  float **KMeanVectors;
  long ItemCountPerCluster[K], VectorClusterCenter[number_of_vectors];
  /**
   * Initialization of K clusters
   */
  KMeanVectors = (float**)malloc(K*sizeof(float*));
  for(long i = 0; i < K; i++) {
    KMeanVectors[i] = (float*)malloc(4*sizeof(float));
    for(int j = 0; j < 4; j++) {
      KMeanVectors[i][j] = VectorMatrix[i][j];
    }
  }
  for(long i = 0; i < number_of_vectors; i++) {
    VectorClusterCenter[i] = rand()%K;
  }

  int reachedConvergence = 0;
  while(!reachedConvergence) {
    reachedConvergence = 1;
    /**
     * Initialize the number of data points in each cluster
     */
    for(long i = 0; i < K; i++) {
      ItemCountPerCluster[i] = 0;
    }

    /**
     * Find nearest cluster for each data item.
     * Returns 0 if the convergence is not reached else 1.
     */
    reachedConvergence *= updateClusterCenters(0, number_of_vectors - 1, VectorMatrix, KMeanVectors, K, ItemCountPerCluster, VectorClusterCenter);
    
    /**
     * Initialize the K mean vectors.
     */
    for(long i = 0; i < K; i++) {
      if(ItemCountPerCluster[i] != 0) {
        for(long j = 0; j < 4; j++) {
          KMeanVectors[i][j] = 0;
        }
      }
    }

    /**
     * Update each mean vector with the summation of all the vectors falling in that cluster.
     * After taking the summation, the mean of clusters are calculated in the followed loop.
     */
    for(long i = 0; i < number_of_vectors; i++) {
      for(long j = 0; j < 4; j++) {
        KMeanVectors[VectorClusterCenter[i]][j] += VectorMatrix[i][j];
      }
    }

    for(long i = 0; i < K; i++) {
      if(ItemCountPerCluster[i] != 0) {
        for(long j = 0; j < 4; j++) {
          KMeanVectors[i][j] /= ItemCountPerCluster[i];
        }
      }
    }
  }

  for(long i = 0; i < K; i++) {
    for(long j = 0; j < 4; j++) {
      printf("%f\t", KMeanVectors[i][j]);
    }
    printf("\n");
  }
  printf("\n");

  return 0;
}