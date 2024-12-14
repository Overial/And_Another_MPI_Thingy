// IDM-23-04
// Afanasyev Vadim
// LR¹3

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include "mpi.h"

using namespace std;

int main()
{
	MPI_Init(NULL, NULL);
	MPI_Finalize();
	return 0;
}
