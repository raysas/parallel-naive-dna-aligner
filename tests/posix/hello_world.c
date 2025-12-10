#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* hello_world(void* _arg){
    int* arg=(int *)_arg;
    printf("hello number %u!\n",*arg);
    pthread_exit(NULL);
}

int main(int argc,char** argv){
    pthread_t thread_ids[10];
    int numbers[10]={10,23,56,35,7,33,24,53,66,10};
    unsigned int i;
    unsigned int rc;

    for (i=0;i<10;i++){
        rc=pthread_create(&thread_ids[i],NULL,hello_world,(void*)&numbers[i]);
        if(rc){
            fprintf(stderr,"error creating thread %d\n",rc);
            exit(EXIT_FAILURE);
        }
    }

    for (i=0;i<10;i++){
        pthread_join(thread_ids[i],NULL);
    }
    printf("done!\n");
}