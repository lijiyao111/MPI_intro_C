/*
Author: Jiyao Li
*/
#include <iostream>
#include <vector>
#include <mpi.h>

using namespace std;

inline int index(int i,int j, int ncols){
    return i*ncols+j;
}

inline int index_s2b(int i,int j, int ncols){
    // given the inner block index, get the true index
    // ncols is the number of column for the inner area
    return (i+1)*(ncols+2)+j+1;
}

int main(){
    const int rows=72, cols=72;  
    const int SOURCE_TAG=101, FIELD_TAG=201;
    const int FROMUPP_TAG = 991, FROMLOW_TAG = 992;
    int npe_wrld, rnk_wrld, master, i, j, count_rows, sender, row_index, sendee;
    int l;

    MPI_Status status;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &npe_wrld);
    MPI_Comm_rank(MPI_COMM_WORLD, &rnk_wrld);

    master = 0;
    int mm=(npe_wrld-1);
    int row_plus = rows +1;
    int low_i, upp_i;

    if(rows/mm <3){
        cout<<"Too many processes! Reduce the number of processes!"<<endl;
        MPI_Abort(MPI_COMM_WORLD,-1);
    }


    // Master code!
    if(rnk_wrld == master){
        cout<<"This is Master "<<rnk_wrld<<'\n';

        double field_init[rows*cols]={0.0}, source[rows*cols]={0.0}, field_final[rows*cols]={0.0};

        // Set the source
        int ny=cols/3;
        for(j=ny; j<2*ny; j++){
            source[index(rows/2,j,cols)] = -1.0;
        }

        // Send source and initial field to slaves
        for(l=0; l<mm; ++l){
            low_i = int((row_plus-1.0)/mm*l);
            upp_i = int((row_plus-1.0)/mm*(l+1));

            // 101 is initial field, 201 is initial source
            MPI_Send(field_init+index(low_i,0,cols),(upp_i-low_i)*cols,MPI_DOUBLE_PRECISION,l+1,FIELD_TAG,MPI_COMM_WORLD);
            MPI_Send(source+index(low_i,0,cols),(upp_i-low_i)*cols,MPI_DOUBLE_PRECISION,l+1,SOURCE_TAG,MPI_COMM_WORLD);
        }

        // Receive the final results from slaves
        for(l=0; l<mm; ++l){
            low_i = int((row_plus-1.0)/mm*l);
            upp_i = int((row_plus-1.0)/mm*(l+1));
            MPI_Recv(field_final+index(low_i,0,cols),(upp_i-low_i)*cols,MPI_DOUBLE_PRECISION,l+1,MPI_ANY_TAG,MPI_COMM_WORLD, &status);
        }


        FILE *fps, *fpf;

        fps=fopen("source_mpi.txt","w");
        fpf=fopen("field_mpi.txt","w");

        for(i=0;i<rows;i++){
            for(j=0; j<cols; j++){
                fprintf(fps, "%12.7f", source[index(i,j,cols)]);
                fprintf(fpf, "%12.7f", field_final[index(i,j,cols)]);
            }
            fprintf(fps, "\n");
            fprintf(fpf, "\n");
        }

        fclose(fps);
        fclose(fpf);




    }
    // Slave Code!
    else{

        low_i = int((row_plus-1.0)/mm*(rnk_wrld-1));
        upp_i = int((row_plus-1.0)/mm*(rnk_wrld));

        int loc_row = upp_i-low_i;
        int loc_col = cols;

        double * loc_field= new double[((upp_i-low_i)+2)*(cols+2)];
        double * loc_source= new double[(upp_i-low_i)*cols];
        double * tmp_field= new double[(upp_i-low_i)*cols];


        // cout<<"This is Slave "<<rnk_wrld<<sizeof(loc_field)/sizeof(loc_field[0])<<' '<<(upp_i-low_i)*cols<<'\n';

        // Receive the sub-block field data from master
        MPI_Recv(tmp_field,loc_row*loc_col,MPI_DOUBLE_PRECISION,master,FIELD_TAG,MPI_COMM_WORLD, &status);
        if(status.MPI_TAG != FIELD_TAG) {
            MPI_Abort(MPI_COMM_WORLD, FIELD_TAG);
        }
        // Receive the sub-block source data from master
        MPI_Recv(loc_source,loc_row*loc_col,MPI_DOUBLE_PRECISION,master,SOURCE_TAG,MPI_COMM_WORLD, &status);
        if(status.MPI_TAG != SOURCE_TAG) {
            MPI_Abort(MPI_COMM_WORLD, SOURCE_TAG);
        }

        for(i=0;i<loc_row;++i){
            for(j=0;j<loc_col;++j){
                loc_field[index_s2b(i,j,loc_col)]=tmp_field[index(i,j,loc_col)];
            }
        }

        // fix the padding row
        for(i=0;i<loc_row;++i){
            loc_field[index_s2b(i,-1,loc_col)]=loc_field[index_s2b(i,0,loc_col)];
            loc_field[index_s2b(i,loc_col,loc_col)]=loc_field[index_s2b(i,loc_col-1,loc_col)];
        }


        struct Neighbor
        {
            int upper, lower;
            int count;
            Neighbor(int x, int y, int n): upper(x),lower(y),count(n) {}


        } ;
        Neighbor nghbr(-1,-1,0);


        // Figure out who is the neighours
        if(rnk_wrld == 1){
            nghbr.lower = rnk_wrld+1;
            nghbr.count = 1;
        }
        else if(rnk_wrld == npe_wrld-1){
            nghbr.upper = rnk_wrld-1;
            nghbr.count = 1;
        }
        else {
            nghbr.upper = rnk_wrld-1;
            nghbr.lower = rnk_wrld+1;
            nghbr.count = 2;
        }


        // Initialize the sending and receiving buffers
        struct Buffer
        {
            double * sendb, * recvb;
        };

        Buffer buff[2]; // 0 is for upperward communication, 1 is for downward communication
        buff[0].sendb = new double[loc_col+2];
        buff[0].recvb = new double[loc_col+2];
        buff[1].sendb = new double[loc_col+2];
        buff[1].recvb = new double[loc_col+2];

        MPI_Request * send_request = new MPI_Request[nghbr.count];
        MPI_Request * recv_request = new MPI_Request[nghbr.count];
        MPI_Status  * send_status = new MPI_Status[nghbr.count]; 
        MPI_Status  * recv_status = new MPI_Status[nghbr.count]; 


        // cout<<"Slave"<<rnk_wrld<<' '<<nghbr.upper<<' '<<nghbr.lower<<'\n';
        const int N_ITER = 5000;
        for(int iter=0; iter<N_ITER;++iter){

            if(nghbr.upper != -1){
                for(j=0;j<loc_col+2;++j){
                    buff[0].sendb[j]=loc_field[index(1,j,loc_col+2)];
                }
            }
            if(nghbr.lower != -1){
                for(j=0;j<loc_col+2;++j){
                    buff[1].sendb[j]=loc_field[index(loc_row,j,loc_col+2)];
                }
            }


            // Non-blocking message send and recieve
            for(l=0;l<2;++l){
                int rs_n = 0;
                if(nghbr.upper != -1){
                    MPI_Isend(buff[0].sendb,loc_col+2,MPI_DOUBLE_PRECISION,nghbr.upper,FROMLOW_TAG,MPI_COMM_WORLD, &send_request[rs_n]);
                    MPI_Irecv(buff[0].recvb,loc_col+2,MPI_DOUBLE_PRECISION,nghbr.upper,FROMUPP_TAG,MPI_COMM_WORLD, &recv_request[rs_n]);
                    rs_n++;
                }
                if(nghbr.lower != -1){
                    MPI_Isend(buff[1].sendb,loc_col+2,MPI_DOUBLE_PRECISION,nghbr.lower,FROMUPP_TAG,MPI_COMM_WORLD, &send_request[rs_n]);
                    MPI_Irecv(buff[1].recvb,loc_col+2,MPI_DOUBLE_PRECISION,nghbr.lower,FROMLOW_TAG,MPI_COMM_WORLD, &recv_request[rs_n]);
                    rs_n++;
                }
            }

            // Check is all the sending and receiving are finished
            MPI_Waitall(nghbr.count,send_request,send_status);
            MPI_Waitall(nghbr.count,recv_request,recv_status);

            // cout<<"Slave "<<rnk_wrld<<" is Here, with "<<nghbr.count<<" Neighbor"<<'\n';

            // Unpack the buffer
            if(nghbr.upper != -1){
                for(j=0;j<loc_col+2;++j){
                    loc_field[index(0,j,loc_col+2)]=buff[0].recvb[j];
                }
            }
            if(nghbr.lower != -1){
                for(j=0;j<loc_col+2;++j){
                    loc_field[index(loc_row+1,j,loc_col+2)]=buff[1].recvb[j];
                }
            }

            // Now is jacobi iteration to update the field
            for(i=0;i<loc_row;++i){
                for(j=0;j<loc_col;++j){
                   tmp_field[index(i,j,loc_col)]=0.25*(loc_field[index_s2b(i-1,j,loc_col)]+loc_field[index_s2b(i+1,j,loc_col)]
                   +loc_field[index_s2b(i,j-1,loc_col)]+loc_field[index_s2b(i,j+1,loc_col)]+loc_source[index(i,j,loc_col)]); 
                }
            }
            for(i=0;i<loc_row;++i){
                for(j=0;j<loc_col;++j){
                   loc_field[index_s2b(i,j,loc_col)]=tmp_field[index(i,j,loc_col)];
                }
            }

        }

        delete [] send_request;
        delete [] send_status;
        delete [] recv_request;
        delete [] recv_status;

        delete [] buff[0].sendb;
        delete [] buff[0].recvb;
        delete [] buff[1].sendb;
        delete [] buff[1].recvb;


        // Send the final calculated subblock field data back to master
        for(i=0;i<loc_row;++i){
            for(j=0;j<loc_col;++j){
                tmp_field[index(i,j,loc_col)]=loc_field[index_s2b(i,j,loc_col)];
            }
        }
        MPI_Send(tmp_field,loc_row*loc_col,MPI_DOUBLE_PRECISION,master,rnk_wrld,MPI_COMM_WORLD);

        delete [] loc_field;
        delete [] loc_source;
        delete [] tmp_field;


    }


    MPI_Finalize();


}