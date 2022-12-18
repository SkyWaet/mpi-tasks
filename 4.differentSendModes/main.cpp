#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <ctime>
#include "mpi.h"
#include <unistd.h>

enum Mode
{
    standart = 1,
    synchronous,
    buffered,
    ready
};

char *generateStringForTest(const int len);
std::string generateRandomString(const int len);

void sendMessage(Mode mode, char *testString, int size);

int main(int argc, char *argv[])
{
    MPI_Status Status;

    MPI_Init(&argc, &argv);

    Mode activeMode = static_cast<Mode>(atoi(argv[1]));
    std::cout << activeMode << std::endl;

    int procRank;

    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    for (int i = 1; i <= 10000000; i *= 10)
    {
        double sendTime = 0;
        double receiveTime = 0;

        for (int j = 0; j < 30; j++)
        {

            if (procRank == 0)
            {
                char *testString = generateStringForTest(i);
                // printf("String to send: %s\n", testString);
                double before = MPI_Wtime();
                MPI_Send(testString, i, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
                double after = MPI_Wtime();
                sendTime += after - before;
                free(testString);
            }
            else
            {
                char *buffer = (char *)malloc(sizeof(char) * (i + 1));
                double before = MPI_Wtime();
                MPI_Recv(buffer, i, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &Status);
                double after = MPI_Wtime();
                receiveTime += after - before;
                // printf("Received string: %s\n", buffer);
                free(buffer);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        if (procRank == 0)
        {
            printf("Mode = %d, Avg send time for string of size %d: %.30f\n", activeMode, i, sendTime / 30);
        }
        else
        {
            printf("Mode = %d, Avg receive time for string of size %d: %.30f\n", activeMode, i, receiveTime / 30);
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
void sendMessage(Mode mode, char *testString, int size)
{
    switch (mode)
    {
    case standart:
        MPI_Send(testString, size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
        break;
    case synchronous:
        MPI_Ssend(testString, size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
        break;
    case buffered:
    {

        int buffSize = sizeof(char) * (size + 1);
        char *bsendBuffer = (char *)malloc(buffSize);
        MPI_Buffer_attach(bsendBuffer, buffSize + MPI_BSEND_OVERHEAD);
        MPI_Bsend(testString, size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
        MPI_Buffer_detach(bsendBuffer, &buffSize);
        free(bsendBuffer);
        break;
    }
    case ready:
        MPI_Rsend(testString, size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
        break;
    default:
        break;
    }
};

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