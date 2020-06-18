#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

void swap(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

int min(int a, int b) {
	return a < b ? a : b;
}

int max(int a, int b) {
	return a > b ? a : b;
}

/**
 * Utility to print and check slice of array.
 * Will be useful when checking each process chunck of array before and after the QuickSort call.
 */
void printArraySlice(int numbers[], int start, int end) {
  printf("Start: %d\t End: %d\n", start, end);
  for (int i = start; i <= end; i++) {
    printf("%d\t", numbers[i]);
  }
  printf("\n\n");
}

/**
 * Creates the partition choosing a pivot element.
 * Returns the partition point for QuickSort.
 */
int Partition(int numbers[], int start, int end) {
  int pivot = numbers[end];
  int i = start - 1;

  for(int j = start; j <= end - 1; j++) {
    if(numbers[j] < pivot) {
      i++;
      swap(&numbers[i], &numbers[j]);
    }
  }

  swap(&numbers[i + 1], &numbers[end]);

  return i + 1;
}

void QuickSort(int numbers[], int start, int end) {
  if(start < end) {
    int partitionPoint = Partition(numbers, start, end);
    QuickSort(numbers, start, partitionPoint - 1);
    QuickSort(numbers, partitionPoint + 1, end);
  }
}

/**
 * Reads the file and returns the number of integers present in the file.
 * This is used to declare the numbers array for QuickSort.
 */
int getFileSize(char inputFileName[]) {
  FILE *fp;
  int size = 0;
  fp = fopen(inputFileName, "r");
  while(!feof(fp)) {
    int tmp;
    fscanf(fp, "%d\n", &tmp);
    size++;
  }
  fclose(fp);

  return size;
}

/**
 * Reads integers from a file and store it in the provided "numbers" array;
 */
void storeNumbersFromFileRead(char inputFileName[], int numbers[], int size) {
  FILE *fp;
  fp = fopen(inputFileName, "r");
  for(int i = 0; i < size; i++) {
    fscanf(fp, "%d\n", &numbers[i]);
  }
  fclose(fp);
}

int main(int argc, char* argv[]) {
  clock_t clock_start, clock_end;
  double cpu_time_used;
  int num_process, my_rank, start, end;
  int root = 0;
  int *numbers;
  int buffCount = 1000000;
  int numbersPerProcess[buffCount], recvcount = buffCount, *sendcounts, *displs, sendCountBackToRoot = 0;
  int size = 0;

  MPI_Init(&argc, &argv); // Initialise MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // Get my rank
  MPI_Comm_size(MPI_COMM_WORLD, &num_process); // Get the total number of processors

  if(my_rank == root) {
  	char inputFileName[100];// = "randomNumbers.txt";
	  size = 0;
	  int endPoints[num_process][2];
    sendcounts = (int*)malloc(num_process*sizeof(int));
    displs = (int*)malloc(num_process*sizeof(int));
	  int cnt = 0;

	  printf("Enter the input file location\n");
	  scanf("%s", inputFileName);

	  size = getFileSize(inputFileName);
    numbers = (int*)malloc(sizeof(int)*size);
	  storeNumbersFromFileRead(inputFileName, numbers, size);

    clock_start = clock();
	  endPoints[0][0] = 0;
	  endPoints[0][1] = size - 1;
	  cnt++;
	  while(cnt < num_process) {
	  	start = endPoints[0][0];
			end = endPoints[0][1]; 
	    int partitionPoint = Partition(numbers, start, end);
	    for(int i = 1; i < cnt; i++) {
	    	endPoints[i - 1][0] = endPoints[i][0];
	    	endPoints[i - 1][1] = endPoints[i][1];
	    }

	    endPoints[cnt - 1][0] = start;
	    endPoints[cnt - 1][1] = max(partitionPoint - 1, start);

	    endPoints[cnt][0] = min(partitionPoint + 1, end);
	    endPoints[cnt][1] = end;

	  	cnt++;
	  }

    // for (int i = 0; i < size; i++)
    // {
    //   printf("%d\t", numbers[i]);
    // }
    // printf("\n");
    
	  for(int chunk = 0; chunk < num_process; chunk++) {
      sendcounts[chunk] = endPoints[chunk][1] - endPoints[chunk][0] + 1;
      displs[chunk] = endPoints[chunk][0];
    }
  }

	MPI_Scatter(sendcounts, 1, MPI_INT, &sendCountBackToRoot, 1, MPI_INT, root, MPI_COMM_WORLD);

  MPI_Scatterv(numbers, sendcounts, displs, MPI_INT, numbersPerProcess, recvcount, MPI_INT, root, MPI_COMM_WORLD);

  // printArraySlice(numbers, 0, size - 1);
  QuickSort(numbersPerProcess, 0, sendCountBackToRoot - 1);
  // printArraySlice(numbersPerProcess, 0, sendCountBackToRoot - 1);

  MPI_Gatherv(numbersPerProcess, sendCountBackToRoot, MPI_INT, numbers, sendcounts, displs, MPI_INT, root, MPI_COMM_WORLD);

  if(my_rank == root) {
    clock_end = clock();
    cpu_time_used = ((double) (clock_end - clock_start)) / CLOCKS_PER_SEC;
    printf("CPU time = %lf\n", cpu_time_used);
    // printArraySlice(numbers, 0, size - 1);
  }

  MPI_Finalize();
  return 0;
}