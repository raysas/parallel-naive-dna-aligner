#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv){


    int rank;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    int* a=malloc(1*sizeof(int));

    MPI_Finalize();
    if ( rank == 0 ){
        for (int i=0; i<10;i++){
            a[i]=i;
        }
    }
}