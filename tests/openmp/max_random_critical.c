#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

int main(int argc, char** argv){
    int i;
    int max_value=0;
    #pragma omp parallel for
        
        for(i=0;i<10;i++){
            int seed; 
            int num;
            seed = time(NULL) + i + omp_get_thread_num();
            num = rand_r(&seed);
            printf("num of thread no %d at iteration %d is %d\n", omp_get_thread_num(),i,num);
            #pragma omp critical
            {
                if (num > max_value){
                    max_value=num;
                    printf("changed max to %d\n",max_value);
                }
            }
        }
}