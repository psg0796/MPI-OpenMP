#include <stdio.h>

void swap(long *a, long *b) {
  long tmp = *a;
  *a = *b;
  *b = tmp;
}

/**
 * Utility to print and check slice of array.
 * Will be useful when checking each process chunck of array before and after the QuickSort call.
 */
void printArraySlice(long numbers[], long start, long end) {
  printf("Start: %ld\t End: %ld\n", start, end);
  for (long i = start; i <= end; i++) {
    printf("%ld\t", numbers[i]);
  }
  printf("\n\n");
}

/**
 * Creates the partition choosing a pivot element.
 * Returns the partition point for QuickSort.
 */
long Partition(long numbers[], long start, long end) {
  long pivot = numbers[end];
  long i = start - 1;

  for(long j = start; j <= end - 1; j++) {
    if(numbers[j] < pivot) {
      i++;
      swap(&numbers[i], &numbers[j]);
    }
  }

  swap(&numbers[i + 1], &numbers[end]);

  return i + 1;
}

void QuickSort(long numbers[], long start, long end) {
  if(start < end) {
    long partitionPoint = Partition(numbers, start, end);
    QuickSort(numbers, start, partitionPoint - 1);
    QuickSort(numbers, partitionPoint + 1, end);
  }
}

/**
 * Reads the file and returns the number of integers present in the file.
 * This is used to declare the numbers array for QuickSort.
 */
long getFileSize(char inputFileName[]) {
  FILE *fp;
  long size = 0;
  fp = fopen(inputFileName, "r");
  while(!feof(fp)) {
    long tmp;
    fscanf(fp, "%ld\n", &tmp);
    size++;
  }
  fclose(fp);

  return size;
}

/**
 * Reads integers from a file and store it in the provided "numbers" array;
 */
void storeNumbersFromFileRead(char inputFileName[], long numbers[], long size) {
  FILE *fp;
  fp = fopen(inputFileName, "r");
  for(int i = 0; i < size; i++) {
    fscanf(fp, "%ld\n", &numbers[i]);
  }
  fclose(fp);
}

int main() {
  char inputFileName[100];
  long size = 0;
  printf("Enter the input file location\n");
  scanf("%s", inputFileName);

  size = getFileSize(inputFileName);
  long numbers[size];
  storeNumbersFromFileRead(inputFileName, numbers, size);

  // printArraySlice(numbers, 0, size - 1);
  QuickSort(numbers, 0, size - 1);
  printArraySlice(numbers, 0, size - 1);
  return 0;
}