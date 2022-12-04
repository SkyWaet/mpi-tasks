#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include "mpi.h"

int getRandomInteger(int lower, int upper);
int *initWithRandomValues(const int size);
int findDotProdSingleThread(int *v1, int *v2, int from, int to);

int main(int argc, char *argv[])
{
    int size = 10;
    int ProcRank, ProcNum, localSum = 0, totalSum = 0;
    int *v1;
    int *v2;
    MPI_Status Status;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if (ProcRank == 0)
    {
        srand(time(0));
        v1 = initWithRandomValues(size);
        v2 = initWithRandomValues(size);
        int singleThreadRes = findDotProdSingleThread(v1, v2, 0, size);
        printf("Single thread ver = %d\n", singleThreadRes);
    }
    MPI_Bcast(v1, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(v2, size, MPI_INT, 0, MPI_COMM_WORLD);

    int k = size / ProcNum;
    int i1 = k * ProcRank;
    int i2 = k * (ProcRank + 1);

    if (ProcRank == ProcNum - 1)
    {
        i2 = size;
    }

    localSum = findDotProdSingleThread(v1, v2, i1, i2);

    MPI_Reduce(&localSum, &totalSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (ProcRank == 0)
    {
        printf("mpi ver = %d\n", totalSum);
    }
    MPI_Finalize();

    return 0;
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

int *initWithRandomValues(const int size)
{
    int *arr = new int[size];
    for (int i = 0; i < size; i++)
    {
        arr[i] = getRandomInteger(-100, 100);
    }
    return arr;
}