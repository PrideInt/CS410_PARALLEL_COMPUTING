/* Torus multicomputer topology of 3^2 processes*/
ARCHITECTURE TORUS(3);

#include <math.h>
#include <stdlib.h>

/* Written and refactored by Pride Yin */

#define m 3
#define p 2

typedef float partition[p + 1][p + 1]; 
partition A[m][m], B[m][m], C[m][m];
partition stream aElements[m][m], stream bElements[m][m];
float stream cElements[m][m];

float finalC[m][m];

int x, y, z;
int ri, ci;
int i, j;

void multiply(int row, int col, value partition localA, value partition localB, partition globalC) {
    /* Multiply function from Section 9.3.4 - COMPLETE PROGRAM */
    int ix, jx, kx, iter, above, left; 

    partition localC = {0};

    if (row > 0) above = row - 1; else above = m - 1; 
    if (col > 0) left = col - 1; else left = m - 1; 

    for (iter = 1; iter <= m; iter++) { 
        send(aElements[row][left], localA);
        send(bElements[above][col], localB);

        for (ix = 1; ix <= p; ix++) {
            for (jx = 1; jx <= p; jx++) {
                for (kx = 1; kx <= p; kx++) {
                    localC[ix][jx] = localC[ix][jx] + localA[ix][kx] * localB[kx][jx];
                }
            }
        }
        recv(aElements[row][col], localA);
        recv(bElements[row][col], localB);
    }
    /* Fix up matrix element positioning */
    int subi = row + 1, subj = col + 1;

    if (subi > m - 1) {
        subi = 0;
    }
    if (subj > m - 1) {
        subj = 0;
    }
    /* Send data to C matrix elements stream so that we can access and print results later */
    send(cElements[subi][subj], localC[1][1] / (p - 1));

    globalC = localC;
}

void init(float *outA, float *outB) {
    float genA = (rand() % 8000) / 1000.0, genB = (rand() % 8000) / 1000.0;

    *outA = genA;
    *outB = genB;
}

void getC(int x, int y, float *element) {
    float res;

    recv(cElements[x][y], res);

    *element = res;
}

main() {
    cout.precision(3);

    /* Initializing our original matrices A and B */
    float matrixA[m][m], matrixB[m][m];

    for (x = 0; x < m; x++) {
        for (y = 0; y < m; y++) {
            FORK init(&matrixA[x][y], &matrixB[x][y]);
        }
    }
    for (x = 0; x < m; x++) for (y = 0; y < m; y++) JOIN;

    /* Partitioning each block A_ij and B_ij accordingly */
    for (x = 0; x < m; x++) {
        for (y = 0; y < m; y++) {
            partition ma = { 0 }, mb = { 0 };

            for (i = 0; i < p; i++) {
                for (j = 0; j < p; j++) {
                    ri = x + i;
                    ci = y + j;
                    if (ri >= m) {
                        ri = ri - m;
                    }
                    if (ci >= m) {
                        ci = ci - m;
                    }
                    ma[i][j] = matrixA[ri][ci];
                    mb[i][j] = matrixB[ri][ci];
                }
            }
            A[x][y] = ma;
            B[x][y] = mb;

            /* ADDITIONAL TESTING TO SEE IF PARTITIONS ARE GOOD:

            cout << "Partition A[" << x << "][" << y << "] = " << endl;
            cout << "[";
            for (i = 0; i < p; i++) {
                for (j = 0; j < p; j++) {
                    cout << A[x][y][i][j] << " ";
                }
                cout << endl;
            }
            cout << "]" << endl;

            cout << "Partition B[" << x << "][" << y << "] = " << endl;
            cout << "[";
            for (i = 0; i < p; i++) {
                for (j = 0; j < p; j++) {
                    cout << B[x][y][i][j] << " ";
                }
                cout << endl;
            }
            cout << "]" << endl;
            */
        }
    }

    cout << "Matrix A:" << endl;
    cout << "[";

    for (x = 0; x < m; x++) {
        for (y = 0; y < m; y++) {
            cout << matrixA[x][y] << " ";
        }
        if (x == m - 1) {
            cout << "]" << endl;
        }
        cout << endl;
    }

    cout << "Matrix B:" << endl;
    cout << "[";
    
    for (x = 0; x < m; x++) {
        for (y = 0; y < m; y++) {
            cout << matrixB[x][y] << " ";
        }
        if (x == m - 1) {
            cout << "]" << endl;
        }
        cout << endl;
    }
    
    /* Perform matrix multiplication operations */
    for (i = 0; i < m; i++) {
        for (j = 0; j < m; j++) {
            FORK (@i * m + j) multiply(i, j, A[i][(j + i) % m], B[(i + j) % m][j], C[i][j]);
        }
    }
    for (i = 0; i < m; i++) for (j = 0; j < m; j++) JOIN;

    for (x = 0; x < m; x++) {
        for (y = 0; y < m; y++) {
            FORK (@x) getC(x, y, &finalC[x][y]);
        }
    }
    for (x = 0; x < m; x++) for (y = 0; y < m; y++) JOIN;

    cout << "Matrix C:" << endl;
    cout << "[";
    
    for (x = 0; x < m; x++) {
        for (y = 0; y < m; y++) {
            cout << finalC[x][y] << " ";
        }
        if (x == m - 1) {
            cout << "]" << endl;
        }
        cout << endl;
    }
} 