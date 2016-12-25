# MPI introduction in C

> Personal notes: MPI vs openMP
> - MPI is for distributed memory or shared memory, deals a lot of message passing. Message passing is more complicated and flexible. 
> - openMP is for shared memory only. OpenMP code contains seriel parts and parallel parts. The paralle parts is indicated by ```#pragma omp parallel``` key words. Message passing is easier to deal with for programmers and mostly behaves behind the scene.
> - MPI basic processing unit is **Process**. openMP basic processing unit is **Thread**.
> - MPI can be compiled using MPICH2 or openMPI package (```mpicc or mpif90``` for C or Fortran). And compiled MPI codes also need to be run with these packages (```mpirun or mpiexec```). 
> - openMP can be compiled using stand C compiler by giving extra compiling flags. For GCC, the flag is ```-fopenmp```. Compiled openMP code runs like normal compiled code.

## hello1.c 
A super simple MPI example, "Hello World", to illustrate simple parallel concepts and MPI environments.
```c
int MPI_Init(int *argc, char ***argv)
int MPI_Comm_size(MPI_Comm comm, int *size)
int MPI_Comm_rank(MPI_Comm comm, int *rank)
int MPI_Get_processor_name(char *name, int *resultlen)
int MPI_Finalize()
```

Compile:
```
mpicc -o hello1.exe hello1.c
```
Run:
```
mpirun -np 8 ./hello1.exe
```

Output:
```
hello from proc = 0 of total 8 processes running on DESKTOP-09SO6NE 
hello from proc = 2 of total 8 processes running on DESKTOP-09SO6NE 
hello from proc = 5 of total 8 processes running on DESKTOP-09SO6NE 
hello from proc = 4 of total 8 processes running on DESKTOP-09SO6NE 
hello from proc = 3 of total 8 processes running on DESKTOP-09SO6NE 
hello from proc = 1 of total 8 processes running on DESKTOP-09SO6NE 
hello from proc = 7 of total 8 processes running on DESKTOP-09SO6NE 
hello from proc = 6 of total 8 processes running on DESKTOP-09SO6NE 
```

**Attention:** There is no gurantee for the order of the processes. And this is a key concept in many other parallel computing, all the processes (or threads) run simultaneously, with no specific order.

## hello2.c 
A slightly modified version of "Hello World". Add MPI_Barrier() function to make all the processes run in order. However, in this example, this goal is achieved by significantly sacrificing the performance. Just to for demonstration purpose.

```c
int MPI_Barrier(MPI_Comm comm)
```

Output:
```
hello from proc = 0 of total 8 processes running on DESKTOP-09SO6NE, time = 0.000000 
hello from proc = 1 of total 8 processes running on DESKTOP-09SO6NE, time = 0.000179 
hello from proc = 2 of total 8 processes running on DESKTOP-09SO6NE, time = 0.000376 
hello from proc = 3 of total 8 processes running on DESKTOP-09SO6NE, time = 0.000598 
hello from proc = 4 of total 8 processes running on DESKTOP-09SO6NE, time = 0.000673 
hello from proc = 5 of total 8 processes running on DESKTOP-09SO6NE, time = 0.000888 
hello from proc = 6 of total 8 processes running on DESKTOP-09SO6NE, time = 0.001391 
hello from proc = 7 of total 8 processes running on DESKTOP-09SO6NE, time = 0.001582 
```

## pi.c
Using integration method to calculate the value of $\pi$. This example is to demonstrate **Collective Communication**.
$$
\int_0^1 \frac{4}{1+x^2} d x = \pi
$$
The parallel computing strategy is to open N processes, and let each process calculte 1/N of the integration. After the partial calculation is finished, gather the results together and perform simple calculation (sum).

The MPI functions used in this code is:
```c
// broadcast from root node to all other processes (including root)
int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype,
    int root, MPI_Comm comm)
// gather the results from all the processes (including root) into root, meanwhile, do simple arithmetic operation specified by MPI_Op
int MPI_Reduce(const void *sendbuf, void *recvbuf, int count,
               MPI_Datatype datatype, MPI_Op op, int root,
               MPI_Comm comm)
```

There are other MPI functions which can perform global communication, MPI_Scatter and MPI_Gather. Scatter function is to send information from on node to many other nodes (can pass different messages to different nodes). Gather function is to gather information from many nodes to one node. 
```c
int MPI_Scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype,
    void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
    MPI_Comm comm)
int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
    void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
    MPI_Comm comm)
```

**Extra note:** Collective communication includes all the processes within the communication group. In order to have collective communication with only a subgroup, ```MPI_Group_excel(MPI_Group group, int n, const int ranks[], MPI_Group *newgroup)``` function could be used to create smaller communication group.

## mat_vec.c
Matrix vector multiplication ($Ax=b$).  This example is to demonstrate **Point-to-point Communication**, which means the source and specificy which destination it want to send the message to.

MPI_Send and MPI_Recv perform **blocking communication**. Blocing vs Nonblocking communication will be discussed in the next example.

```c
// Send message from current process to dest, with tag
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest,
    int tag, MPI_Comm comm)
// Receive message at current process from source, with specific tag (use MPI_ANY_TAG if don't want to specify tag)
int MPI_Recv(void *buf, int count, MPI_Datatype datatype,
    int source, int tag, MPI_Comm comm, MPI_Status *status)
```

In this code, **master-slave** paradigm was implemented. And this code used a fancy master-slave strategy. 

Rank 0 is used as **master** to orgnize the communication, which sends each row of the matrix to different **slave** processes. Assuming the number of rows is bigger than the number of slave processes, once any slave process finished calculating one row, it return the result to the master. Meanwhile, after the master receives the result, and if there are remaining rows to process, it will send additional row to the idle process to work on. If there is no remaining row, master send tag '-999' (or any user-specified number) to tell the slave process to stop.

## poisson_mpi.cpp 
Solve 2D Poisson equation in an iterative way. This iterative method can be easily modified to solve other more complicated **PDE**, e.g. heat equation, wave equation. 

Poisson equation $\frac{\partial^2 u(x,y)}{\partial x^2} + \frac{\partial^2 u(x,y)}{\partial y^2} + f(x,y) =0$ can be solved using Jabobi iteration as:

$$
u(i,j) := \frac{[u(i-1,j)+u(i+1,j)+u(i,j-1)+u(i,j+1)+f(x,y)/h^2]}{4}
$$

This is a much more advanced parallel computing, usually called **Domain Decomposition**. The 2D matrix will be broken into several adjacent blocks. Each process will process one block. The challenge is that when numerically calculate the derivative for the boundary layer, an extra layer is required from the neighbouring block. However, without message communication, each process does not have more information other than the block it owns. 

Because there are more information to be passed and received between the processes, and each process needs to do both sending and receiving using the same code, blocking communication won't work in this case. Think about this:
one process is doing sending first, followed by receiving. Its neighbour is also doing sending first then receiving. If we use blocking communication, none of them will move on to the receiving part. Both of them will insist that they will start to receive only if the sending is finish, which is deadlock. 

**Blocking communication** is done using ```MPI_Send()``` and ```MPI_Recv()```. These functions do not return (i.e., they block) until the communication is finished. Simplifying somewhat, this means that the buffer passed to ```MPI_Send()``` can be reused, either because MPI saved it somewhere, or because it has been received by the destination. Similarly, ```MPI_Recv()``` returns when the receive buffer has been filled with valid data.

In contrast, **non-blocking** communication is done using ```MPI_Isend()``` and ```MPI_Irecv()```. These function return immediately (i.e., they do not block) even if the communication is not finished yet. You must call ```MPI_Wait()``` or ```MPI_Probe()``` to see whether the communication has finished.

```c
// non-blocking send
int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest,
    int tag, MPI_Comm comm, MPI_Request *request)
// non-blocking receive
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype,
        int source, int tag, MPI_Comm comm, MPI_Request *request)
// check if all the non-blocking send and receive are finished, before using the passed messages to do any calculation
int MPI_Waitall(int count, MPI_Request array_of_requests[],
    MPI_Status *array_of_statuses)
```

The parallel computing strategy implemented in this code is that the rank 0 is the **master** node, which organize job assignment, initial condition passing and final result collection. All other processes are **slave**, which does the jacobi iteration to solve poisson equation. **blocking communication** happens between master and slaves. **nonblocking communication** happens between the slaves.

**poisson_mpi.cpp** code generates poisson equation solution using MPI parallel computing. **poisson_serial.c** code is a serial code to calculate the poisson equation solution, for comparison. 

**plot_poisson_mpi.py** code read source_mpi.txt as the output from MPI code and source.txt as the output from the serial code and compare them. My tests show that the difference is 0, which means my MPI parallel code run correctly.


