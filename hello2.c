/*
Author: Jiyao Li
*/
#include <mpi.h>
#include <stdio.h>

int main(){

    int npe_wrld, rnk_wrld, name_len;
    double timstart, timend;
    char proc_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(NULL, NULL); // Initialize MPI environment

    MPI_Comm_size(MPI_COMM_WORLD, &npe_wrld); // determine world size (total number of processes)
    MPI_Comm_rank(MPI_COMM_WORLD, &rnk_wrld); // determine rank of this process
    MPI_Get_processor_name(proc_name, &name_len); // get local processor name

    timstart = MPI_Wtime(); // get current time

    // int n;
    for(int n=0; n<npe_wrld; n++){

        if(rnk_wrld == n){
            timend = MPI_Wtime();
            printf("hello from proc = %d of total %d processes running on %s, time = %f \n",
                rnk_wrld,npe_wrld,proc_name,timend-timstart);
        } 

        MPI_Barrier(MPI_COMM_WORLD); // every process stop here to wait for everyone else to reach this point, then move on
    }

    MPI_Finalize(); // Finalize MPI environment

}
