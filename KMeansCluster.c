#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

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

void storeNumbersFromFileRead(char inputFileName[], float **numbers, int size) {
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

float getEuclideanDistance(float A[], float B[], int size) {
  float dist = 0;
  for(int i = 0; i < size; i++) {
    dist += ((A[i] - B[i])*(A[i] - B[i]));
  }
  return dist;
}

int getNearestCluster(float *VectorMatrix, float **KMeanVectors, int K) {
  int minK = 0;
  float minDistance = getEuclideanDistance(VectorMatrix, KMeanVectors[0], 4);
  for(int k = 1; k < K; k++) {
    float distance = getEuclideanDistance(VectorMatrix, KMeanVectors[k], 4);
    if(distance < minDistance) {
      minDistance = distance;
      minK = k;
    }
  }
  return minK;
}

int updateClusterCenters(int start, int end, float **VectorMatrix, float **KMeanVectors, int K, int ItemCountPerCluster[], int VectorClusterCenter[]) {
  int reachedConvergence = 1;

  for(int i = start; i <= end; i++) {
    int minK = getNearestCluster(VectorMatrix[i], KMeanVectors, K);

    ItemCountPerCluster[minK]++;

    if(VectorClusterCenter[i] != minK) {
      reachedConvergence = 0;
      VectorClusterCenter[i] = minK;
    }
  }

  return reachedConvergence;
}

int main(int argc, char **argv) {
  int number_of_vectors = 0;
  int my_rank, num_process, root = 0;
  int K;
  double elapsed_time;

  MPI_Init(&argc, &argv); // Initialise MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // Get my rank
  MPI_Comm_size(MPI_COMM_WORLD, &num_process); // Get the total number of processors

  number_of_vectors = getNumberOfVectorsInFile(INPUT_FILE);

  if(number_of_vectors % num_process != 0) {
    printf("Please enter number of processes such that it is a factor of \t%d\n Sorry for inconvenience.\n", number_of_vectors);
    exit(0);
  }

  float **VectorMatrix;
  VectorMatrix = (float**)malloc(number_of_vectors*sizeof(float*));
  for(int i = 0; i < number_of_vectors; i++) {
    VectorMatrix[i] = (float*)malloc(4*sizeof(float));
  }
  
  storeNumbersFromFileRead(INPUT_FILE, VectorMatrix, number_of_vectors);

  if(my_rank == 0) {
    printf("Enter the value of K: ");
    scanf("%d", &K);
  }

  MPI_Bcast(&K, 1, MPI_INT, root, MPI_COMM_WORLD);

  float **KMeanVectors;
  int ItemCountPerCluster[K], VectorClusterCenter[number_of_vectors];
  /**
   * Initialization of K clusters
   */
  KMeanVectors = (float**)malloc(K*sizeof(float*));
  for(int i = 0; i < K; i++) {
    KMeanVectors[i] = (float*)malloc(4*sizeof(float));
    for(int j = 0; j < 4; j++) {
      KMeanVectors[i][j] = VectorMatrix[i][j];
    }
  }
  for(int i = 0; i < number_of_vectors; i++) {
    VectorClusterCenter[i] = rand()%K;
  }

  int reachedConvergence = 0;

  int number_of_vectors_per_process = number_of_vectors/num_process, start = my_rank * number_of_vectors_per_process, end = start + number_of_vectors_per_process;

  elapsed_time = - MPI_Wtime();
  while(!reachedConvergence) {
    int reachedConvergenceLocal = 1;
    /**
     * Initialize the number of data points in each cluster
     */
    int ItemCountPerClusterLocal[K];
    for(int i = 0; i < K; i++) {
      ItemCountPerClusterLocal[i] = 0;
    }

    /**
     * Find nearest cluster for each data item.
     * Returns 0 if the convergence is not reached else 1.
     */
    reachedConvergenceLocal = updateClusterCenters(start, end - 1, VectorMatrix, KMeanVectors, K, ItemCountPerClusterLocal, VectorClusterCenter);

    MPI_Allreduce(&reachedConvergenceLocal, &reachedConvergence, 1, MPI_INT, MPI_PROD, MPI_COMM_WORLD);

    for(int i = 0; i < K; i++) {
      MPI_Allreduce(&ItemCountPerClusterLocal[i], &ItemCountPerCluster[i], 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    }    

    /**
     * Initialize the K mean vectors.
     */
    float KMeanVectorsLocal[K][4];

    for(int i = 0; i < K; i++) {
      if(ItemCountPerCluster[i] != 0) {
        for(int j = 0; j < 4; j++) {
          KMeanVectorsLocal[i][j] = 0;
        }
      }
    }

    /**
     * Update each mean vector with the summation of all the vectors falling in that cluster.
     * After taking the summation, the mean of clusters are calculated in the followed loop.
     */
    for(int i = start; i < end; i++) {
      for(int j = 0; j < 4; j++) {
        KMeanVectorsLocal[VectorClusterCenter[i]][j] += VectorMatrix[i][j];
      }
    }

    for(int i = 0; i < K; i++) {
      for(int j = 0; j < 4; j++) {
        MPI_Allreduce(&KMeanVectorsLocal[i][j], &KMeanVectors[i][j], 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
      }
    }

    for(int i = 0; i < K; i++) {
      if(ItemCountPerCluster[i] != 0) {
        for(int j = 0; j < 4; j++) {
          KMeanVectors[i][j] /= ItemCountPerCluster[i];
        }
      }
    }
  }

  elapsed_time += MPI_Wtime();
  MPI_Finalize();

  if(my_rank == 0) {
    printf("Time taken for K Mean clusters calculation\t%lf\n\n", elapsed_time);

    for(int i = 0; i < K; i++) {
      for(int j = 0; j < 4; j++) {
        printf("%f\t", KMeanVectors[i][j]);
      }
      printf("\n");
    }
    printf("\n");
  }

  return 0;
}