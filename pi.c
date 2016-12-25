/*
Author: Jiyao Li
*/
#include <stdio.h>
#include <mpi.h>
#include <math.h>


int main(){
    int npe_wrld, rnk_wrld;
    int n=0;
    double pi_true=3.14159265358979323864;
    double del_x, x_left, pi_piece, pi_approx;
    double time_start, time_end, x;

    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &npe_wrld); // determine world size (total number of processes)
    MPI_Comm_rank(MPI_COMM_WORLD, &rnk_wrld); // determine rank of this process
    // MPI_Get_processor_name(proc_name, &name_len); // get local processor name

    if(rnk_wrld == 0){
        printf("Enter the total number of intervals: ");
        scanf("%d", &n);
        printf("%d\n", n);
    }

    // printf("%d\n",rnk_wrld );

    MPI_Bcast(&n, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);

    time_start = MPI_Wtime();

    del_x = 1.0 / n;
    x_left = (double) rnk_wrld / npe_wrld;
    pi_piece = 0.0;

    for(int i=1; i<=n/npe_wrld;i++){
        x=x_left+del_x*(i-0.5);
        pi_piece += del_x*(4.0/(1.0+x*x));
    }

    MPI_Reduce(&pi_piece, &pi_approx, 1, MPI_DOUBLE_PRECISION, MPI_SUM, 0, MPI_COMM_WORLD);

    time_end = MPI_Wtime();

    if(rnk_wrld == 0){
        printf("error in calculating pi = %.20f\n",fabs(pi_approx-pi_true));
    }

    MPI_Barrier(MPI_COMM_WORLD);

    printf("time = %f in process %d \n", time_end - time_start, rnk_wrld );

    MPI_Finalize();



}



