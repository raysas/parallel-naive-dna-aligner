#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 8

// ----------- starters for DNA sequence generation ------------

#define DNA_LENGTH 1000 * 1000
#define MOTIF_LENGTH 1000

typedef enum nucleotide {
    A,
    C,
    G,
    T
} nucleotide_type;

nucleotide_type random_nucleotide(){
    int r = rand() % 4;
    return (nucleotide_type)r; 
    // 0:A, 1:C, 2:G, 3:T
}

void generate_random_dna_sequence(nucleotide_type* dna_sequence, long length){
    for(size_t i=0; i<length; i++){
        dna_sequence[i] = random_nucleotide();
    }
}


// -------------------------------------------------------------


// -------------- starters for POSIX threads -------------------

typedef struct thread_data{
    unsigned int thread_id;
    unsigned long* shared_max_value_pointer; // pointer to global max number of matches
    unsigned int* shared_max_index_pointer;  // pointer to global max index
    pthread_mutex_t* mutex_max_lock;  // pointer to mutex lock
    
    // -- the rest is okay
    unsigned int start_index;
    unsigned int end_index;
    nucleotide_type* dna_sequence;
    nucleotide_type* motif;
} thread_data;

void* find_local_max(void* _arg){
    thread_data* arg=(thread_data*)_arg;

    unsigned long i;
    unsigned long j;

    // -- define local max variables --
    unsigned long local_max_value=0;
    unsigned int local_max_index=0;

    for(i=arg->start_index;i<arg->end_index;i++){
        unsigned long num_matches=0;
        for(j=0;j<MOTIF_LENGTH;j++){
            if(arg->dna_sequence[i+j]==arg->motif[j]){
                num_matches++;
            }
        }
        if (num_matches>local_max_value){
            local_max_value=num_matches;
            local_max_index=i;
        }
    }

    // -- critical section to compare and update global max --
    pthread_mutex_lock(arg->mutex_max_lock);
    if (local_max_value > *(arg->shared_max_value_pointer)){
        *(arg->shared_max_value_pointer)=local_max_value;
        *(arg->shared_max_index_pointer)=local_max_index;
    }
    pthread_mutex_unlock(arg->mutex_max_lock);

    pthread_exit(NULL);
}


// -------------------------------------------------------------

int main(int argc,char** argv){
    nucleotide_type* dna_sequence = malloc(DNA_LENGTH * sizeof(nucleotide_type));
    nucleotide_type* motif = malloc(MOTIF_LENGTH * sizeof(nucleotide_type));

    // -- global max variables will be shared accross all threads in thread_data struct --
    unsigned long global_max_value=0;
    unsigned int global_max_index;
    // -- INIT MUTEX LOCK --
    pthread_mutex_t mutex_max_lock=PTHREAD_MUTEX_INITIALIZER;


    unsigned long i;
    unsigned int rc;

    pthread_t thread_ids[NUM_THREADS];
    thread_data local_search[NUM_THREADS];

    generate_random_dna_sequence(dna_sequence, DNA_LENGTH);
    generate_random_dna_sequence(motif, MOTIF_LENGTH);

    unsigned long chunk_size = (DNA_LENGTH - MOTIF_LENGTH + 1) / NUM_THREADS;
    unsigned long remainder = (DNA_LENGTH - MOTIF_LENGTH + 1) % NUM_THREADS;


    /* -- loop 1: pthread_create --*/
    for(i=0;i<NUM_THREADS;i++){
        local_search[i].thread_id = i;
        local_search[i].start_index = i * chunk_size;
        
        // -- will follow last thread takes remainder strategy --
        if (i == NUM_THREADS - 1) {
            local_search[i].end_index = (i + 1) * chunk_size + remainder; // last thread takes remainder
        } else {
            local_search[i].end_index = (i + 1) * chunk_size;
        }

        local_search[i].shared_max_value_pointer = &global_max_value;
        local_search[i].shared_max_index_pointer = &global_max_index;
        local_search[i].mutex_max_lock = &mutex_max_lock;


        local_search[i].dna_sequence = dna_sequence;
        local_search[i].motif = motif;

        rc=pthread_create(&thread_ids[i],NULL,find_local_max,(void *)&local_search[i]);
        if(rc){
            printf("error creating thread %d\n",rc);
            return -1;
        }
    }

    /* -- loop 2: pthread_join --*/
    for(i=0;i<NUM_THREADS;i++){
        pthread_join(thread_ids[i],NULL);
    }

    printf("Global max matches: %lu at index %u\n", global_max_value, global_max_index);

    free(dna_sequence);
    free(motif);
    return 0;
}