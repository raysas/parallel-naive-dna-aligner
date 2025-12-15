#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>


int main(int argc, char** argv){
    int size;   
    int rank;   

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);// default to 1
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);// default to 0

    printf("hello from size (number of ranks?) %d, rank (task no) %d\n",size,rank);

    MPI_Finalize();
    return EXIT_SUCCESS;
}