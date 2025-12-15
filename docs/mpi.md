# MPI

**disributed programming**: independent processes running on different data performing common task through communication via network
(message passing facilitates communication through network)

MPI (message passing interface) : standardized and portable message-passing system designed to function on a wide variety of parallel computing architectures

## Code
 
To run an mpi program can run it thorugh `mpicc -np <num_processes> <program_name.c> -o <output_name>`, would only need to have add `#include <mpi.h>` at the start of the program and compile it with `mpicc` instead of `gcc`

The code comprising the MPI part should be between:

```c
MPI_Init(*argc,*argv); 
//..
MPI_Finalize();
```

```c
MPI_Comm_rank(MPI_COMM_WORLD, &rank); //get the rank of the process in var rank
MPI_Comm_size(MPI_COMM_WORLD, &size); //get the total number of processes in var size
```

### Sending and receiving messages

MPI_Send and MPI_Receive are blocking communication functions used in MPI to send and receive messages between processes in a distributed computing environment (blocking because no other operations can be performed until the send or receive operation is completed)

```c
MPI_Send(
    void* data_buffer, //pointer to data to be sent
    int count, //number of elements in the buffer
    MPI_Datatype datatype, //data type of each element
    int dest_rank, //rank of destination process
    int tag, //message tag
    MPI_Comm communicator //communicator
);
```

```c
MPI_Receive(
    void* data_buffer, //pointer to buffer to store received data
    int count, //number of elements in the buffer
    MPI_Datatype datatype, //data type of each element
    int source_rank, //rank of source process
    int tag, //message tag
    MPI_Comm communicator, //communicator
    MPI_Status* status //status object to store information about the received message
);
```

> [!NOTE]
> The MPI_Status object can be set to NULL if the status information is not needed, in fact its a struct that has 3 info: source (rank of the sending process), tag (message tag), error (error code)


*e.g.*:

```c
/* will be:
sending a messgae "1" from process rank 0 to process of rank 1 and the tag fo the message is 3
*/
int rank;
int buffer_0=1; //message is this variable
int buffer_1

int dest_rank=1;
int tag=3;

MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);

if (rank == 0) {
    MPI_Send(
        &buffer_0,
        1,
        MPI_INT,
        dest_rank,
        tag,
        MPI_COMM_WORLD
    );
}

if ( rank == 1 ){
    MPI_Receive(
        &buffer_1,
        1,
        MPI_INT,
        0,
        3,
        MPI_COMM_WORLD,
        NULL
    )
}

MPI_Finalize();
```

*for every send there should be a receive*


### Example for Send and Receive

sending in a cycle *0->1, 1->2... n-2 -> n-1, n-1 -> 0*

In here should define a source and destination for each current process such that:
* source is tright before it: source = (rank - 1 + size) % size;*
* destination is tright after it: dest = (rank + 1) % size;

As for sending and receiving, should split the processes in two groups:
* even ranked processes send first then receive
* odd ranked processes receive first then send

This is very essential to avoid deadlocks

```c
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);

// -- for each current defining source to receive from and destination send to
int source = (rank - 1 + size) % size;
int dest = (rank + 1) % size;

int send_buffer=rank;
int rec_buffer;

if (rank % 2 == 0 ){
    MPI_Send(&send_buffer, 1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Receive(&rec_buffer,1,MPI_INT,source,0,MPI_COMM_WORLD,NULL);
}
else{
    MPI_Receive(&rec_buffer, 1, MPI_INT, source, 0, MPI_COMM_WORLD,NULL);
    MPI_Send(&send_buffer, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
}
MPI_Finalize();
```

### Collective communication

**whats collective communication?**
multiple processes working together in dat atransfer, collective calculation, or synchronization operation

**Broadcasting**: sending from one process to all other processes

```c
MPI_Brcast(
    *buffer_to_be_broadcasted, //* for pointer to data buffer
    count,
    data_type,
    root_process_rank,
    MPI_COMM_WORLD
)
```
Now all processes will have the same data in their buffer

**Gather:** collecting data from all processes to a single root process

```c
MPI_Gather(
    *send_buffer,
    send_count,
    send_data_type,
    *recv_buffer, //array that will store all of them
    recv_count,
    recv_data_type,
    root_process_rank,
    MPI_COMM_WORLD
)
```

**Reduce:** performing a reduction operation (like sum, max, min) on data from all processes and storing the result in a single root process

```c
MPI_Reduce(
    *send_buffer,
    *recv_buffer,
    count,
    data_type,
    operation, //like MPI_SUM, MPI_MAX
    root_process_rank,
    MPI_COMM_WORLD
)
```