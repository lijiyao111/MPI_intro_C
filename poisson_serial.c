#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    int n0, n, nx, ny;
    int i, j, l, iblk_max, jblk_max;
    int iter;
    double h;
    double ** alph, ** beta, **tmpry;

    n0=72;
    n=n0+2;

    iblk_max=3;
    jblk_max=3;

    alph= (double **) malloc(n * sizeof(double *));
    beta= (double **) malloc(n * sizeof(double *));
    tmpry= (double **) malloc(n * sizeof(double *));

    for(i=0; i<n; i++){
        alph[i] = (double *) malloc(n*sizeof(double));
        beta[i] = (double *) malloc(n*sizeof(double));
        tmpry[i] = (double *) malloc(n*sizeof(double));
    }

    for(i=0; i<n; i++){
        for(j=0; j<n; j++){
            alph[i][j]=0.0;
            beta[i][j]=0.0;
            tmpry[i][j]=0.0;
        }
    }

    // location of Source
    nx=n0/iblk_max;
    ny=n0/jblk_max;
    for(j=ny+1; j<2*ny+1; j++){
        beta[n0/2+1][j] = -1.0;
    }

    printf("Before iterations!\n") ;

    iter=5000;

    for(l=0; l<iter; l++){

        for(i=1;i<n-1;i++){
            for(j=1; j<n-1; j++){
                tmpry[i][j]=(alph[i-1][j]+alph[i+1][j]+alph[i][j-1]+alph[i][j+1]
                    +beta[i][j])/4.0;
            }
        }
        for(i=0;i<n;i++){
            for(j=0; j<n; j++){
                alph[i][j]=tmpry[i][j];
            }
        }
    };

    FILE *fps, *fpf;

    fps=fopen("source.txt","w");
    fpf=fopen("field.txt","w");

    for(i=1;i<n-1;i++){
        for(j=1; j<n-1; j++){
            fprintf(fps, "%12.7f", beta[i][j]);
            fprintf(fpf, "%12.7f", alph[i][j]);
        }
        fprintf(fps, "\n");
        fprintf(fpf, "\n");
    }

    fclose(fps);
    fclose(fpf);

for(i=0;i<n;i++){
    free(alph[i]);
    free(beta[i]);
    free(tmpry[i]);
}
    free(alph);
    free(beta);
    free(tmpry);




    return 0;
}