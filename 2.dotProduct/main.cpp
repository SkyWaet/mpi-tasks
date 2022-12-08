#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include "mpi.h"

int getRandomInteger(int lower, int upper);
void fillWithRandomValues(int *arr, int size);
int getLeftBorder(int batchSize, int rank);
int getRightBorder(int batchSize, int rank, int procNum, int size);
int findDotProdSingleThread(int *v1, int *v2, int from, int to);

int main(int argc, char *argv[])
{
    MPI_Status status;
    MPI_Init(&argc, &argv);
    int size;
    if (atoi(argv[1]) == 0)
    {
        size = 100;
    }
    else if (atoi(argv[1]) == 1)
    {
        size = 100000;
    }
    else
    {
        size = 100000000;
    }

    double startTime = 0;
    double endTime = 0;

    int ProcRank, ProcNum, totalSum;

    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    int *left = (int *)malloc(sizeof(int) * size);
    int *right = (int *)malloc(sizeof(int) * size);
    int batchSize = size / ProcNum;

    int leftBorder = getLeftBorder(batchSize, ProcRank);
    int rightBorder = getRightBorder(batchSize, ProcRank, ProcNum, size);

    if (ProcRank == 0)
    {
        srand(time(0));
        fillWithRandomValues(left, size);
        fillWithRandomValues(right, size);
        double stStartTime = MPI_Wtime();
        int stResult = findDotProdSingleThread(left, right, 0, size);
        double stEndTime = MPI_Wtime();
        printf("Single thread result: %d, elapsed time: %0.15f, array size %d\n", stResult, stEndTime - stStartTime, size);
        startTime = MPI_Wtime();
        for (int i = 1; i < ProcNum; i++)
        {
            int threadLeftBorder = getLeftBorder(batchSize, i);
            int threadRightBorder = getRightBorder(batchSize, i, ProcNum, size);
            MPI_Send(left + threadLeftBorder, threadRightBorder - threadLeftBorder, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(right + threadLeftBorder, threadRightBorder - threadLeftBorder, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(left + leftBorder, rightBorder - leftBorder, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(right + leftBorder, rightBorder - leftBorder, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }

    int localSum = findDotProdSingleThread(left, right, leftBorder, rightBorder);

    int reduceResult = MPI_Reduce(&localSum, &totalSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (reduceResult != MPI_SUCCESS)
    {
        printf("Rank %d error in reduce: %d\n", ProcRank, reduceResult);
    }

    if (ProcRank == 0)
    {
        endTime = MPI_Wtime();
        printf("Multi-Thread result: %d, elapsed time: %.15f, number of threads: %d\n", totalSum, endTime - startTime, ProcNum);
    }
    free(left);
    free(right);
    MPI_Finalize();
    return 0;
}

int getLeftBorder(int batchSize, int rank)
{
    return batchSize * rank;
}

int getRightBorder(int batchSize, int rank, int procNum, int size)
{
    if (rank == procNum - 1)
    {
        return size;
    }

    return batchSize * (rank + 1);
}

int findDotProdSingleThread(int *v1, int *v2, int from, int to)
{
    int sum = 0;
    for (int i = from; i < to; i++)
    {
        sum += v1[i] * v2[i];
    }
    return sum;
}

int getRandomInteger(int lower, int upper)
{
    return std::rand() % (upper - lower + 1) + lower;
}

void fillWithRandomValues(int *arr, int size)
{
    for (int i = 0; i < size; i++)
    {
        arr[i] = getRandomInteger(-1000, 1000);
    }
}