#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include "mpi.h"

int getRandomInteger(int lower, int upper);
void fillWithRandomValues(int *arr, int size);
int findMinSingleThread(int *array, int from, int to);

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

    int ProcRank, ProcNum, totalMin;

    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    int *array = (int *)malloc(sizeof(int) * size);
    int k = size / ProcNum;
    int i1 = k * ProcRank;
    int i2 = k * (ProcRank + 1);

    if (ProcRank == ProcNum - 1)
    {
        i2 = size;
    }

    if (ProcRank == 0)
    {
        srand(time(0));
        fillWithRandomValues(array, size);
        double stStartTime = MPI_Wtime();
        int stResult = findMinSingleThread(array, 0, size);
        double stEndTime = MPI_Wtime();
        printf("Single thread result: %d, elapsed time: %0.15f, array size %d\n", stResult, stEndTime - stStartTime, size);
        startTime = MPI_Wtime();
        for (int i = 1; i < ProcNum; i++)
        {
            MPI_Send(array + i1, i2 - i1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(array + i1, i2 - i1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }

    int localMin = findMinSingleThread(array, i1, i2);

    int reduceResult = MPI_Reduce(&localMin, &totalMin, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    if (reduceResult != MPI_SUCCESS)
    {
        printf("Rank %d error in reduce: %d\n", ProcRank, reduceResult);
    }

    if (ProcRank == 0)
    {
        endTime = MPI_Wtime();
        printf("Multi-Thread result: %d, elapsed time: %.15f, number of threads: %d\n", totalMin, endTime - startTime, ProcNum);
    }
    free(array);
    MPI_Finalize();
    return 0;
}

int findMinSingleThread(int *array, int from, int to)
{
    int min = array[from];
    for (int i = from + 1; i < to; i++)
    {
        if (array[i] < min)
        {
            min = array[i];
        }
    }
    return min;
}

int getRandomInteger(int lower, int upper)
{
    return std::rand() % (upper - lower + 1) + lower;
}

void fillWithRandomValues(int *arr, int size)
{
    for (int i = 0; i < size; i++)
    {
        arr[i] = getRandomInteger(-100, 100);
    }
}