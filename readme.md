# Parallel Aligner

Practice exercise on performing the *naive dna alignment* in parallel using pthreads, openmp, and MPI. This exercise is part of the Advanced Programming course, part II given by prof. LUCAS @ saclay 2025-2026

> disclaimer: this is practice repo that is not corrected or optimized, just for learning purposes and might actually have mistakes as it's a personal take on the exercise

## Problem

### naive dna alignment

![naive dna aln](./assets/naive-dna.png)

**Given:**  
* DNA sequence: e.g., "ACGTACGTGACG"
* motif sequence: e.g., "ACA"

**Return:** dna subsequence with highest number of matches with motif, through:
* position of this motif (start index)
* number of matches of this motif (max value)

Sequentially, this can be solved in O(n*m) time, where n is the length of the DNA sequence and m is the length of the motif, through a simple nested loop.  
The challenge however is to trasnform yet a simple algorithm into an efficient parallel one in C

### starters in c

For this problem we need to define some variables and types, in the example here the dna sequences will be randomly generated

* dna seq length (to be randomly generated - otherwise provide a fixed dna sequence)
* motif length (to be randomly generated - otherwise provide a fixed motif)
* nucleotide type (enum, i.e. A,C,G,T)
* function to generate nucleotide randomly (p=0.25 for each)
* function to randomly generate a dna sequence (based on previous function + length)

*will be using the long type for lengths*

```c
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
```

### posix threads

To parallelize using ptrheads, the strategy followed here:  
* split the dna sequence into chunks, one per thread
* each thread searches for the local max in its chunk
* main thread collects local maxes and finds the global max

**`thread_data` struct:**   
Shoudl save things like thread id, local max value and index for sure (similar to max_random example), in addition, need to define the parition of teh problem this thread will work on, that's by providing access to global dna sequence and motif as struct members, as well as start and end indices for each thread to work on its chunk (thread boundaries defined)
```c
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
```


**`void* find_local_max(void* _arg)` function:**  
This function should implement local max through a loop over the assigned chunk from teh large dna seq, and a double loop to go over the motif and count matches, updating local max value and index as needed, at the end will have one local max per thread saved in the thread_data struct
```c
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
```

**main:**  
3 loops are needed in main:
* loop 1: create threads with `pthread_create`, passing each thread its chunk through start and end indices in thread_data struct
* loop 2: join threads with `pthread_join`
* loop 3: collect local maxes from each thread and find the global max

> [!NOTE]
> *chunk size will be defined as DNA_LENGTH / NUM_THREADS, as for the remainder, will be added to the last thread's chunk*


This part's [c code in `src/naive_dna_posix.c`](./src/naive_dna_posix.c)

### posix threads with mutex

Same strategy as before with a bit of tweaking at the end:
* [x] split the dna sequence into chunks, one per thread
* [x] each thread searches for the ~local~ max in its chunk
* [ ] ~main thread collects local maxes and finds the global max~

In the local max search function `find_local_max`, at the end of the function where it has found its local max, we need to compare it with the global max that is shared accross all threads, and update it if needed.  
This is a tricky step to perform as we can run into a _race condition*_, as in several threads might be performing the comparison concurrently in an unsafe manner that results in overwriting results.

Mutex locks will be isntalled for the rescue, the trick is to lock the comparison and assignment operations in such a way only one thread performs them (in an atomic fashion) at a time.

mutex lock:
* add a `pthread_mutex_t*` pointer variable to thread_data struct to pass the mutex to each thread
* make `max_value` and `max_index` pointers in thread_data making tehm shareable accross all threads
* define a mutex variable in main using `pthread_mutex_t mutex_max_lock;` and initialize it with `PTHREAD_MUTEX_INITIALIZER`
* pass mutex variable to each thread through thread_data struct in main's first loop (ptrhead_create loop)
* add lock and unlock calls around the critical section that compares and assign a max value in `find_local_max` function


**`thread_data` struct:**    
will add `pthread_mutex_t` pointer, will aslo make `shared_max_value_pointer` and `shared_max_index_pointer` instead of having local copies, so that each thread can update the global max directly (maybe better this way?)
```c
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
```


**`void* find_local_max(void* _arg)` function:**  
The function here defines a local max variable within the function's scope, then after finding it and exiting the loop, it locks mutex, compares it to the max value pointer saved by the thread_data struxt that is SHARED ACCROSS ALL THREADS, and updates it if needed, then unlocks the mutex => ensuring only one thread at a time can perform this critical section
```c
void* find_local_max(void* _arg){
    thread_data* arg=(thread_data*)_arg;

    unsigned long i;
    unsigned long j;
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

    pthread_mutex_lock(arg->mutex_max_lock);
    if (local_max_value > *(arg->shared_max_value_pointer)){
        *(arg->shared_max_value_pointer)=local_max_value;
        *(arg->shared_max_index_pointer)=local_max_index;
    }
    pthread_mutex_unlock(arg->mutex_max_lock);

    pthread_exit(NULL);
}
```

**main:**  
2 loops are needed in main:
* loop 1: create threads with `pthread_create`, passing each thread its chunk through start and end indices in thread_data struct AS WELL AS the mutex and pointers to global max variables (shared accross all threads)
* loop 2: join threads with `pthread_join`

This way, no need for a 3rd loop in main to collect local maxes, as the global max is updated directly by each thread safely using mutex locks.

> [!NOTE]
> running both `naive_dna_posix.c` and `naive_dna_posix_mutex.c` versions in src/ (compiled in `./posix` and `./posix_mutex` respectively) shows that both give the same result: "Global max matches: 322 at index 793254"

### OpenMP

### MPI


## Notes

Code in C for the problem is found in `src/`, other codes and practice exercises are found in `tests/`.  
Written notes can be found in `docs/`

## TODO

- [ ] make it take user input for dna and motif sequences (try it with rosalind datasets: [SUBS problem](https://rosalind.info/problems/subs/))