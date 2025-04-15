ARCHITECTURE HYPERCUBE(7);

#include <mpi.h>
#include <stdlib.h>

/* Written and refactored by Pride Yin */

/* Scaling down the dimensions of the image to 768x768 to avoid storage overflow complications */
#define n 768
/* #define n 3 */

/* Used to calculate n, given n = m * 2^7 */
/* In this case: m = 6; 768 = 6 * 2^7 */
#define m 6

int inRows[m + 2][n + 2], outRows[m][n]; 
int filter[3][3] = {1, 1, 1, 1, 1, 1, 1, 1, 1};

MPI_Status status;

int rank, totalProcesses, size;
int i, j, k;

int origin, destination;
int tag;

void inputImage() {
    /* Overflow issues hinge on the size of this image. However because
       we are only on process 0, we can limit the amount of memory allocated
       when in MPI mode utilizing multicomputer topology as to avoid duplication
       as opposed to keeping this data structure global.
    */
    int image[n + 2][n + 2];

    for (i = 0; i < n + 2; i++) {
        for (j = 0; j < n + 2; j++) {
            image[i][j] = rand() % 256;
        }
    }

    for (k = 0; k < totalProcesses; k++) { 
        destination = k;
        tag = 1;

        MPI_Send(&image[k * m][0], size, MPI_INT, destination, tag, MPI_COMM_WORLD);
    }
}

main() {
    MPI_Init();
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &totalProcesses);

    size = (m + 2) * (n + 2);

    if (rank == 0) {
        inputImage();
    }
    origin = 0;
    tag = 1;

    MPI_Recv(&inRows[0][0], size, MPI_INT, origin, tag, MPI_COMM_WORLD, &status);
    
    /* Filter mask */
    for (i = 1; i <= m; i++) {
        for (j = 1; j <= n; j++) {
            outRows[i - 1, j - 1] = 
                (int) ((filter[0, 0] * inRows[i - 1, j - 1] 
                + filter[0, 1] * inRows[i - 1, j]
                + filter[0, 2] * inRows[i - 1, j + 1]
                + filter[1, 0] * inRows[i, j - 1]
                + filter[1, 1] * inRows[i, j]
                + filter[1, 2] * inRows[i, j + 1]
                + filter[2, 0] * inRows[i + 1, j - 1]
                + filter[2, 1] * inRows[i + 1, j]
                + filter[2, 2] * inRows[i + 1, j + 1]) / 9);
        }
    }
    size = m * n;

    destination = 0;
    tag = 2;

    MPI_Send(&outRows[0][0], size, MPI_INT, destination, tag, MPI_COMM_WORLD);

    if (rank == 0) { 
        for (k = 0; k < totalProcesses; k++) { 
            origin = k;
            tag = 2;

            MPI_Recv(&outRows[0][0], size, MPI_INT, origin, tag, MPI_COMM_WORLD, &status);

            for (i = 0; i < m; i++) {
                for (j = 0; j <= n; j++) {
                    /* Only printing out the first 15 values of each row to avoid line buffer overflow */

                    if (j <= 15) {
                        cout << outRows[i][j];
                    }
                }
                cout << endl; 
            }
        }
    }
    MPI_Finalize();
}