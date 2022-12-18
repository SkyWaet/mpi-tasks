#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <ctime>
#include "mpi.h"
#include <unistd.h>

char *generateStringForTest(const int len);
std::string generateRandomString(const int len);
double exchangeWithSimpleMethods(int messageLength, int procRank);
double exchangeWithSendRecv(int messageLength, int procRank);

int main(int argc, char *argv[])
{

    MPI_Init(&argc, &argv);

    int mode = atoi(argv[1]);
    int procRank;

    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
    srand(procRank * 100000 + MPI_Wtime());

    for (int i = 10; i <= 10000000; i *= 10)
    {
        double messageExchangeTime = 0;

        for (int j = 0; j < 30; j++)
        {
            messageExchangeTime += mode == 1
                                       ? exchangeWithSimpleMethods(i, procRank)
                                       : exchangeWithSendRecv(i, procRank);
        }

        if (procRank == 0)
        {
            printf("Avg exchange time from for strings of size %d: %.30f\n", i, messageExchangeTime / 30);
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
double exchangeWithSimpleMethods(int messageLength, int procRank)
{
    MPI_Status Status;
    char *testString = generateStringForTest(messageLength);
    char *buffer = (char *)malloc(sizeof(char) * (messageLength + 1));
    double messageExchangeTime;
    if (procRank == 0)
    {
        // printf("String sent by 0: %s\n", testString);
        double beforeSend = MPI_Wtime();
        MPI_Send(testString, messageLength, MPI_CHAR, 1, 0, MPI_COMM_WORLD);

        MPI_Recv(buffer, messageLength, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &Status);
        double afterRecv = MPI_Wtime();
        // printf("String recieved by 0: %s\n", buffer);
        messageExchangeTime += afterRecv - beforeSend;
    }
    else
    {
        MPI_Recv(buffer, messageLength, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &Status);
        // printf("String received by 1: %s\n", buffer);

        // printf("String sent by 1: %s\n", testString);
        MPI_Send(testString, messageLength, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    free(testString);
    free(buffer);
    return messageExchangeTime;
}

double exchangeWithSendRecv(int messageLength, int procRank)
{
    MPI_Status Status;
    char *testString = generateStringForTest(messageLength);
    double messageExchangeTime;
    if (procRank == 0)
    {
        // printf("String sent by 0: %s\n", testString);
        double beforeSend = MPI_Wtime();
        MPI_Sendrecv_replace(testString, messageLength + 1, MPI_CHAR, 1, 0, 1, 0, MPI_COMM_WORLD, &Status);
        double afterRecv = MPI_Wtime();
        // printf("String recieved by 0: %s\n", testString);
        messageExchangeTime += afterRecv - beforeSend;
    }
    else
    {
        // printf("String received by 1: %s\n", buffer);
        MPI_Sendrecv_replace(testString, messageLength + 1, MPI_CHAR, 0, 0, 0, 0, MPI_COMM_WORLD, &Status);
        // printf("String sent by 1: %s\n", testString);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    free(testString);

    return messageExchangeTime;
}
char *generateStringForTest(const int len)
{
    char *testString = (char *)malloc(sizeof(char) * (len + 1));
    std::string randomString = generateRandomString(len);
    strcpy(testString, randomString.c_str());
    return testString;
}

std::string generateRandomString(const int len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
    {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}