#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int mails=0;
// int lock=0; //thre is smtg in pthread to use

pthread_mutex_t mutex;

void* routine(){
    for (int i=0;i<1000000;i++){


        // if (lock==1){
        //     //wait until the lock is 0

        // }
        // lock=1; //set it to 1 when we re doinf smtg
        // -- all of this is one function work:
        pthread_mutex_lock(&mutex);
        
        mails++; // only one thread will execute this line of code when locked
        //use it only when necessary
        pthread_mutex_unlock(&mutex);

        // lock=0;//when done doinf smtg
        // read mails
        // inccreament
        // write it back to memory -> write mails
        // -- its fine if on 1 thread
        // -- on multiple threads doesnt work here
        // -- RACE CONDITION
    }
}

int main(int agrc, char* argv[]){

    pthread_t t1;
    pthread_t t2;

    pthread_mutex_init(&mutex,NULL); //initializing the mutex

    if(pthread_create(&t1,NULL,&routine,NULL)!=0){
        return 1;
    }
    
    if(pthread_create(&t2,NULL,&routine,NULL)!=0){
        return 2;
    }

    if (pthread_join(t1,NULL)!=0){
        return 3;
    }

    if(pthread_join(t2,NULL)!=0){
        return 4;
    }

    pthread_mutex_destroy(&mutex);

    printf("number of mails: %d\n",mails);
    return 0;
}