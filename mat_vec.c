#include <stdio.h>
#include <mpi.h>
#include <math.h>


int main(){
    const int rows=300, cols=5;  
    int npe_wrld, rnk_wrld, master, i, j, count_rows, sender, row_index, sendee;
    int Msendcount=0, Mreceivcount=0, Ssendcount=0, Sreceivcount=0;
    double A[rows][cols],b[cols],c[rows],buffer[cols];
    double ans, time_start, time_end;
    MPI_Status status;


    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &npe_wrld);
    MPI_Comm_rank(MPI_COMM_WORLD, &rnk_wrld);

    time_start = MPI_Wtime();
    master = 0;

// MASTER Program!
    if(rnk_wrld == master){
        printf("Master %d \n", rnk_wrld);        
        for(int j =0; j<cols;j++){
            b[j] = j+1;
        }
        for(int i=0; i<rows; i++){
            for(int j=0; j<cols; j++){
                A[i][j] = i+j; 
            }
        }

        MPI_Bcast(b, cols, MPI_DOUBLE_PRECISION, master, MPI_COMM_WORLD);

        count_rows = 0;
        for(int i=0; i<npe_wrld-1;i++){

            sendee = i+1;
            if(count_rows < rows){
                for (int j = 0; j < cols; ++j)
                {
                    buffer[j]=A[i][j];
                }
                MPI_Send(buffer,cols,MPI_DOUBLE_PRECISION,sendee,i,MPI_COMM_WORLD);
                printf("Master sends to Slave %d with row %d and Tag %d\n", sendee, i, i);
                count_rows += 1;
            }
            else{
                MPI_Send(MPI_BOTTOM,0,MPI_DOUBLE_PRECISION,sendee,999,MPI_COMM_WORLD);
                printf("Master sends to Slave %d with None and Tag 999\n", sendee);
            }
        }


        for(int i=0; i<rows;i++){
            MPI_Recv(&ans,1,MPI_DOUBLE_PRECISION,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD, &status);
        printf("Master receives from Slave %d with row %d and Tag %d\n",status.MPI_SOURCE, status.MPI_TAG, status.MPI_TAG );
            sender = status.MPI_SOURCE;
            row_index = status.MPI_TAG;
            c[row_index]=ans;
            // printf("count_rows %d, rows %d\n",count_rows, rows);

            if(count_rows < rows){
                for(int j=0;j<cols;j++){
                    buffer[j]=A[count_rows][j];
                }
                MPI_Send(buffer,cols,MPI_DOUBLE_PRECISION,sender,count_rows,MPI_COMM_WORLD);
                printf("Master sends to Slave %d with row %d and Tag %d\n", sender, count_rows, count_rows);
                count_rows +=1;
            }
            else{
                MPI_Send(MPI_BOTTOM,0,MPI_DOUBLE_PRECISION,sender,999,MPI_COMM_WORLD);
                printf("Master sends to Slave %d with None and Tag 999\n", sender);
            }
        }

        time_end = MPI_Wtime();

        // printf("Master elapsed CPU time %f\n", time_end - time_start);
        MPI_Barrier(MPI_COMM_WORLD);

        printf("Matrix A:\n");
        for(int i=0; i<rows; i++){
            for(int j=0; j<cols; j++){
                printf("%.1f ",A[i][j]);
            }
            printf("\n");
        }

        printf("Vector b:\n");
        for(int j=0;j<cols;j++){
            printf("%.1f\n",b[j] );
        }

        printf("Result of A x b:\n");
        for(int i=0;i<rows;i++){
            printf("%.1f\n",c[i] );
        }

    }    
    else{
        MPI_Bcast(b, cols, MPI_DOUBLE_PRECISION, master, MPI_COMM_WORLD);

        // printf("Slave %d \n", rnk_wrld);        

        while(1){
            MPI_Recv(buffer,cols,MPI_DOUBLE_PRECISION,master,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
            printf("Slave %d receives from Master with row %d and TAG %d\n",rnk_wrld, status.MPI_TAG,status.MPI_TAG );
            if(status.MPI_TAG == 999){
                MPI_Barrier(MPI_COMM_WORLD);
                break;
            } 

            row_index=status.MPI_TAG;
            ans =0.0;
            for(int j=0; j<cols; j++){
                ans += buffer[j]*b[j];
            }
            MPI_Send(&ans,1,MPI_DOUBLE_PRECISION,master,row_index,MPI_COMM_WORLD);
            printf("Slave %d sends to Master with row %d and TAG %d\n",rnk_wrld,status.MPI_TAG, status.MPI_TAG );
        }
        time_end = MPI_Wtime();
        // printf("Slave elapsed CPU time %f\n",time_end - time_start);


    }

    MPI_Finalize();



  }