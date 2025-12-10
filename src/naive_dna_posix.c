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
    unsigned long max_value; // number of matches
    unsigned int max_index;  // index of where max occurs
    
    // -- need to define start and end indices for each thread --
    unsigned int start_index;
    unsigned int end_index;

    // -- need to pass the global DNA sequence and motif --
    nucleotide_type* dna_sequence;
    nucleotide_type* motif;
} thread_data;

void* find_local_max(void* _arg){
    thread_data* arg=(thread_data*)_arg;
    // -- implementing local max search here --
    unsigned long i;
    unsigned long j;

    for(i=arg->start_index;i<arg->end_index;i++){
        unsigned long num_matches=0;
        for(j=0;j<MOTIF_LENGTH;j++){
            if(arg->dna_sequence[i+j]==arg->motif[j]){
                num_matches++;
            }
        }
        if (num_matches>arg->max_value){
            arg->max_value=num_matches;
            arg->max_index=i;
        }
    }

    pthread_exit(NULL);
}

// -------------------------------------------------------------

int main(int argc,char** argv){
    nucleotide_type* dna_sequence = malloc(DNA_LENGTH * sizeof(nucleotide_type));
    nucleotide_type* motif = malloc(MOTIF_LENGTH * sizeof(nucleotide_type));
    unsigned long global_max_value=0;
    unsigned int global_max_index;
    unsigned long i;
    unsigned int rc;

    pthread_t thread_ids[NUM_THREADS];
    thread_data local_search[NUM_THREADS];


    generate_random_dna_sequence(dna_sequence, DNA_LENGTH);
    generate_random_dna_sequence(motif, MOTIF_LENGTH);

    // -- divide sequence chunks among threads by defining start and end indices --
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

        local_search[i].max_value = 0;
        local_search[i].max_index = 0;
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

    /* -- loop 3: find global max from local maxes --*/
    for(i=0;i<NUM_THREADS;i++){
        if(local_search[i].max_value > global_max_value){
            global_max_value = local_search[i].max_value;
            global_max_index = local_search[i].max_index;
        }
    }

    printf("Global max matches: %lu at index %u\n", global_max_value, global_max_index);

    free(dna_sequence);
    free(motif);
    return 0;
}