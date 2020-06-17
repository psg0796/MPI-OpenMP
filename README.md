# MPI-OpenMP

############################  Question 1  #########################################

Question. Generate N random numbers and store it in a file. Write a sequential program in C to
sort N numbers (read from the file) in ascending order using Quick sort.

Solution: Files -> randomNumberGenerator.c & QuickSort.c
  1.  randomNumberGenerator.c
      This file generates N random numbers and stores it in a file randomNumbers.txt.

    Compile: gcc -o randomNumberGenerator.out randomNumberGenerator.c
    Run: ./randomNumberGenerator.out

  2.  QuickSort.c
      This files takes a file name as user input and prints the result of QuickSort on stdout.
    
    Compile: gcc -o QuickSort.out QuickSort.c
    Run: ./QuickSort.out

###################################################################################

############################  Question 2  #########################################

Question. Iris dataset (iris.data) is given to you. The fifth column of the data file contain
class name. There are 3 classes. Ignore the last column and consider only 4 columns.
Consider each row as a data vector of 4 values (i.e. 4-dimensional vector). There are
around 150 rows (150 vectors) in the data file. Write a sequential program in C to
compute the mean vector and covariance matrix.

Solution: Files -> MeanVectorAndCovariance.c
  1.  MeanVectorAndCovariance.c
      Prints the Mean Vector and Covariance Vector. The program uses iris.data file present in the
      folder. To change the input file location change the INPUT_FILE macro defined at the top in this file.

    Compile: gcc -o MeanVectorAndCovariance.out MeanVectorAndCovariance.c
    Run: ./MeanVectorAndCovariance.out

###################################################################################

############################  Question 3  #########################################

Question. Consider the Iris dataset as in question 2. Write a sequential program in C to implement
K-means clustering algorithm. Read the value of K from the user. The output of the
program should be the cluster centres.

Solution: Files -> KMeansCluster.c
  1.  KMeansCluster.c
      This file takes K as a user input and prints K cluster points to stdout.
      The input file can be changed by changing the INPUT_FILE macro defined at the top in this file.

    Compile: gcc -o KMeansCluster.out KMeansCluster.c
    Run: ./KMeansCluster.out

###################################################################################
