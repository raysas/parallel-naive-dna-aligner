#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

// same but for loop the threads

int mails=0;
pthread_mutex_t mutex;

void* routine(){
    for (int i=0;i<1000000;i++){

        pthread_mutex_lock(&mutex);
        
        mails++;

        pthread_mutex_unlock(&mutex);
    }
}

int main(int agrc, char* argv[]){

    pthread_t th[4];
    int i;

    pthread_mutex_init(&mutex,NULL); //initializing the mutex

    for (i=0;i<4;i++){
        if (pthread_create(&th[i], NULL, &routine, NULL)!=0){
            perror("failed to create thread");
            return 1;
            /// -- btw th+i is the same as &th[i]
        }
        printf("thread %d has started\n",i);
    }
    for (i=0;i<4;i++){
        if (pthread_join(th[i],NULL)!=0){ 
            return 2;
        }
        printf("thread %d has finished\n",i);

    }

    pthread_mutex_destroy(&mutex);

    printf("number of mails: %d\n",mails);
    return 0;
}