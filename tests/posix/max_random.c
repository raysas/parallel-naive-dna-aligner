#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define NUM_THREADS 8
#define NUM_NUMBERS 1000 * 1000 * 1000


typedef struct {
    unsigned int id;
    unsigned int numbers_per_thread;
    unsigned int local_max;
} thread_data;

void* find_local_max(void* _arg){
    thread_data* arg=(thread_data *) _arg;

    int j;
    int seed=time(NULL) + arg->id;

    for (j=0;j<arg->numbers_per_thread;j++){
        unsigned int rand_value=rand_r(&seed);

        if(rand_value>arg->local_max){
            arg->local_max=rand_value;
        }
    }

    pthread_exit(NULL);
}

int main(int argc,char **argv){

    pthread_t thread_ids[NUM_THREADS];
    thread_data local_maxes[NUM_THREADS];
    unsigned int i;
    unsigned int global_max=0;
    int rc;

    int numbers_per_thread=NUM_NUMBERS/NUM_THREADS;
    int remainder=NUM_NUMBERS%NUM_THREADS;

    for (i=0;i<NUM_THREADS;i++){
        local_maxes[i].id=i;
        local_maxes[i].local_max=0;
        
        // -- distributing the remainder on the last thread
        if (i==NUM_THREADS-1){
            local_maxes[i].numbers_per_thread=numbers_per_thread + remainder;
        } else{
            local_maxes[i].numbers_per_thread=numbers_per_thread;
        }

        rc=pthread_create(&thread_ids[i],NULL,find_local_max,&local_maxes[i]);
        if (rc){
            fprintf(stderr,"Error creating thread\n");
            exit(EXIT_FAILURE);
        }
    }

    for (i=0;i<NUM_THREADS;i++){
        pthread_join(thread_ids[i],NULL);
    }

    for(i=0;i<NUM_THREADS;i++){
        if(local_maxes[i].local_max>global_max){
            global_max=local_maxes[i].local_max;
        }
    }

    printf("global max: %u\n",global_max);
    exit(EXIT_SUCCESS);

}