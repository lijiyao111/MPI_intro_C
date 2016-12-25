/*
Author: Jiyao Li
*/
#include <mpi.h>
#include <stdio.h>

int main(){

    int npe_wrld, rnk_wrld, name_len;
    char proc_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(NULL, NULL); // Initialize MPI environment

    MPI_Comm_size(MPI_COMM_WORLD, &npe_wrld); // determine world size (total number of processes)
    MPI_Comm_rank(MPI_COMM_WORLD, &rnk_wrld); // determine rank of this process
    MPI_Get_processor_name(proc_name, &name_len); // get local processor name

    printf("hello from proc = %d of total %d processes running on %s \n",rnk_wrld,npe_wrld,proc_name);

    MPI_Finalize(); // Finalize MPI environment

}
