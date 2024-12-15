// IDM-23-04
// Afanasyev Vadim
// LR№3

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include "mpi.h"

using namespace std;
using namespace std::chrono;

#define SIZE 1000
#define TAG_NA_DERZHI 0

void PrintMatrix(int* Matrix)
{
    for (int i = 0; i < SIZE; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            cout << setw(4) << Matrix[i * SIZE + j] << "|";
        }
        cout << endl;
    }
}

void FillMatrix(int* mat)
{
    for (int i = 0; i < SIZE * SIZE; ++i) {
        mat[i] = rand() % 10;
    }
}

void DeleteMatrix(int*& mat)
{
    delete[] mat;
}

int main() {
    {
        MPI_Init(NULL, NULL);

        int ProcessCount, Rank;
        MPI_Comm_size(MPI_COMM_WORLD, &ProcessCount);
        MPI_Comm_rank(MPI_COMM_WORLD, &Rank);

        int* MatrixA = nullptr;
        int* MatrixB = nullptr;
        int* MatrixC = nullptr;

        srand(static_cast<unsigned int>(time(NULL)));
        steady_clock::time_point StartTime = high_resolution_clock::now();

        // Фул закуп на главном процессе
        if (Rank == 0) {
            MatrixA = new int[SIZE * SIZE];
            MatrixB = new int[SIZE * SIZE];
            MatrixC = new int[SIZE * SIZE];

            FillMatrix(MatrixA);
            FillMatrix(MatrixB);

            // Отладочная инициализация матриц

            /*MatrixA[0] = 1;
            MatrixA[1] = 7;
            MatrixA[2] = 4;
            MatrixA[3] = 0;
            MatrixA[4] = 9;
            MatrixA[5] = 4;
            MatrixA[6] = 8;
            MatrixA[7] = 8;
            MatrixA[8] = 2;
            MatrixA[9] = 4;
            MatrixA[10] = 5;
            MatrixA[11] = 5;
            MatrixA[12] = 1;
            MatrixA[13] = 7;
            MatrixA[14] = 1;
            MatrixA[15] = 1;

            MatrixB[0] = 5;
            MatrixB[1] = 2;
            MatrixB[2] = 7;
            MatrixB[3] = 6;
            MatrixB[4] = 1;
            MatrixB[5] = 4;
            MatrixB[6] = 2;
            MatrixB[7] = 3;
            MatrixB[8] = 2;
            MatrixB[9] = 2;
            MatrixB[10] = 1;
            MatrixB[11] = 6;
            MatrixB[12] = 8;
            MatrixB[13] = 5;
            MatrixB[14] = 7;
            MatrixB[15] = 6;*/

            // Ожидаемый результат:
            // 69 | 45 | 60 | 72
            // 126| 90 | 129| 162
            // 57 | 54 | 57 | 81
            // 22 | 37 | 29 | 39
        }

        int ProcessColCount = SIZE / ProcessCount;
        int ProcessElementCount = ProcessColCount * SIZE;

        int* BlockMatrixA = new int[ProcessElementCount] { 0 };
        int* BlockMatrixB = new int[ProcessElementCount] { 0 };
        int* BlockMatrixC = new int[ProcessElementCount] { 0 };

        // Раздаем пацанам фрагменты матриц
        MPI_Scatter(MatrixA, ProcessElementCount, MPI_INT, BlockMatrixA, ProcessElementCount, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatter(MatrixB, ProcessElementCount, MPI_INT, BlockMatrixB, ProcessElementCount, MPI_INT, 0, MPI_COMM_WORLD);

        // Пацаны пилят таски
        for (int Process = 0; Process < ProcessCount; ++Process) {
            int CurrentIndex = (Rank + Process) % ProcessCount;
            
            for (int i = 0; i < ProcessColCount; ++i) {
                
                for (int j = 0; j < SIZE; ++j) {
                    int Buffer = 0;
                    
                    for (int k = 0; k < ProcessColCount; ++k) {
                        Buffer += BlockMatrixA[i * SIZE + k + CurrentIndex * ProcessColCount] * BlockMatrixB[k * SIZE + j];
                    }
                    
                    BlockMatrixC[i * SIZE + j] += Buffer;
                }
            }

            // You shall not pass
            MPI_Barrier(MPI_COMM_WORLD);

            // Сваливаем таски на других
            MPI_Sendrecv_replace(BlockMatrixB, ProcessElementCount, MPI_INT,
                (Rank + 1) % ProcessCount,
                TAG_NA_DERZHI,
                (Rank - 1 + ProcessCount) % ProcessCount,
                TAG_NA_DERZHI,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        MPI_Gather(BlockMatrixC, ProcessElementCount, MPI_INT, MatrixC, ProcessElementCount, MPI_INT, 0, MPI_COMM_WORLD);

        steady_clock::time_point EndTime = high_resolution_clock::now();
        long long TotalTime = duration_cast<microseconds>(EndTime - StartTime).count();

        if (Rank == 0) {
            cout << "MPI MODE USING " << ProcessCount << " WORKER PROCESSES" << endl;

            cout << "Matrix A:" << endl;
            PrintMatrix(MatrixA);

            cout << "Matrix B:" << endl;
            PrintMatrix(MatrixB);

            std::cout << "Result matrix C:" << std::endl;
            PrintMatrix(MatrixC);

            cout << "Total time: " << TotalTime << endl;
        }

        // Не оставляем следов

        DeleteMatrix(BlockMatrixA);
        DeleteMatrix(BlockMatrixB);
        DeleteMatrix(BlockMatrixC);

        DeleteMatrix(MatrixA);
        DeleteMatrix(MatrixB);
        DeleteMatrix(MatrixC);

        // Всем спасибо, все свободны
        MPI_Finalize();
    }

    // Уходя, гасите всех
    _CrtDumpMemoryLeaks();

    return 0;
}
