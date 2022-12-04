#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include "mpi.h"

int getRandomInteger(int lower, int upper);
int *initWithRandomValues(const int size);
int findMinSingleThread(int *array, int from, int to);

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int size = 10;

    int ProcRank, ProcNum, totalMin;

    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
    int *array;
    if (ProcRank == 0)
    {
        srand(time(0));
        array = initWithRandomValues(size);
        totalMin = array[0];
        int singleThreadRes = findMinSingleThread(array, 0, size);
        printf("Single thread ver = %d\n", singleThreadRes);
    }

    MPI_Bcast(array, size, MPI_INT, 0, MPI_COMM_WORLD);

    int k = size / ProcNum;
    int i1 = k * ProcRank;
    int i2 = k * (ProcRank + 1);

    if (ProcRank == ProcNum - 1)
    {
        i2 = size;
    }
    int localMin = findMinSingleThread(array, i1, i2);
    MPI_Reduce(&localMin, &totalMin, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    if (ProcRank == 0)
    {
        printf("mpi ver = %d\n", totalMin);
    }
    MPI_Finalize();

    return 0;
}

int findMinSingleThread(int *array, int from, int to)
{
    int min = array[0];
    for (int i = from; i < to; i++)
    {
        min = std::min(min, array[i]);
    }
    return min;
}

int getRandomInteger(int lower, int upper)
{
    return std::rand() % (upper - lower + 1) + lower;
}

int *initWithRandomValues(const int size)
{
    int *arr = new int[size];
    for (int i = 0; i < size; i++)
    {
        arr[i] = getRandomInteger(-100, 100);
    }
    return arr;
}