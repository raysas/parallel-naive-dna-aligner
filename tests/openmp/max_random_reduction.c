#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>



int main(int argc, char** argv){
    int max_value;
    int i;
    #pragma omp parallel for reduction(max:max_value) num_threads(4)
    for(i=0;i<10;i++){
        int seed=time(NULL)+i;
        max_value=rand_r(&seed);
        printf("max_value %d in thread %d at iteration %d\n",max_value,omp_get_thread_num(),i);
    }

    printf("max at the end is %d\n",max_value);
}