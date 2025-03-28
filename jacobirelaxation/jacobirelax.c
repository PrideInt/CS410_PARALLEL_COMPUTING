#include <math.h> 

/* Parallelized implementation of Jacobi Relaxation with: */
/* Convergence testing and local synchronization employing iteratively implemented aggregation barriers. */

/*
    Improved performance sees an approximately 57% increase in speedup compared to
    general parallelized Jacobi Relaxation without efficiency
    by implementing aggregation barriers along with convergence to see major
    improvements in speedup, additionally with minor optimizations by
    establishing local synchronization by minimizing the amount
    of operations relative to the duration of computation.

    SPEEDUP of General Parallelized Jacobi Relaxation: 15.48; n = 32
    SPEEDUP of Binary Tree Parallelized Jacobi Relaxation: 16.23; n = 32
        (relative to numiter): 50
    SPEEDUP of Performant Parallelized Jacobi Relaxation: 27.84; n = 32

    The code was refactored to include the necessary functions
    and implementations needed to achieve the desired improvements in performance.
*/

/* Written/Refactored by Pride Yin */

#define n 32
#define tolerance .01
/* #define numiter 50 */

float A[n + 2][n + 2], B[n + 2][n + 2]; 
int i, j;
/* int k; */

/* int stream channel[n]; */
int stream higher[n + 1], stream lower[n + 1];

int count; 
spinlock arrival, departure; 
boolean global;

/*
void treeBarrier(int i) {
    /* Refactored code from section 6.4 - BINARY TREE IMPLEMENTATION OF BARRIERS */

    /*
    int position, dummy; 
   
    position = 1;

    while (i / position % 2 == 0 && position < n)  {
      recv(channel[i], dummy);   
      position = position * 2; 
    }
    if (i != 0) { 
      send(channel[i - position], 1);  
      recv(channel[i], dummy);
    }
    while (position > 1)  { 
      position = position / 2; 
      send(channel[i + position], 1);  
    }
}
*/

void localBarrier(int i) {
    /* Refactored code from section 6.5 - LOCAL SYNCHRONIZATION */
    
    int dummy;

    if (i > 1) {
        send(higher[i - 1], 1);
    }
    if (i < n) { 
        send(lower[i + 1], 1);
        recv(higher[i], dummy);
    } 
    if (i > 1) {
        recv(lower[i], dummy);
    }
}

boolean aggregate(boolean local) {
    /* Refactored code from section 6.6.2 - IMPLEMENTING PARALLEL AGGREGATION */

    boolean result;

    Lock(arrival); 
    count = count + 1; 
    global = global && local;

    if (count < n) {
        Unlock(arrival);
    } else {
        Unlock(departure);
    }
    Lock(departure); 
    count = count - 1; 
    result = global;
    
    if (count > 0) {
        Unlock(departure);
    } else { 
        Unlock(arrival);   
        global = true;
    } 
    return result; 
}

main() { 
    cout.precision(4);

    for (i = 0; i <= n + 1; i++)  {   
        for (j = 0; j <= n + 1; j++) {
            cin >> A[i][j];
        } 
    }

    /* GENERAL PARALLEL JACOBI RELAXATION
        Uncomment numiter and k.

    B = A; 
    for (k = 1; k <= numiter; k++) {
        forall i = 1 to n do { 
            int j; 
            for (j = 1; j <= n; j++) { 
                B[i][j] = (A[i-1][j] + A[i+1][j] + A[i][j-1] + A[i][j+1]) / 4; 
            }
        }
        forall i = 1 to n do A[i] = B[i];   
    }
    */

    /* BINARY TREE PARALLEL JACOBI RELAXATION
        Uncomment numiter, k, channel and treeBarrier().

    B = A; 
    forall i = 1 to n do { 
        int j, k;
        
        for (k = 1; k <= numiter; k++) {
            for (j = 1; j <= n; j++) {
                B[i][j] = (A[i-1][j] + A[i+1][j] + A[i][j-1] + A[i][j+1]) / 4; 
            }
            treeBarrier(i - 1);
            A[i] = B[i];
            treeBarrier(i - 1);
        }
    }
    */

    /* PERFORMANT PARALLEL JACOBI RELAXATION */
    count = 0;
    Unlock(arrival);  
    Lock(departure);  
    global = true;

    B = A; 
    forall i = 1 to n do {
        int j; 
        float change, maxChange; 
        boolean done; 

        do {  
            maxChange = 0; 
            for (j = 1; j <= n; j++) {
                B[i][j] = (A[i - 1][j] + A[i + 1][j] + A[i][j - 1] + A[i][j + 1]) / 4; 

                change = fabs(B[i][j] - A[i][j]); 

                if (change > maxChange) {
                    maxChange = change;
                }
                localBarrier(i); 
                A[i] = B[i];
                localBarrier(i);
            } 
            done = aggregate(maxChange < tolerance);
        } while (!done);
    }

    for (i = 0; i <= n + 1; i++) {
        cout << "Row " << i << ": " << endl;
        for (j = 0; j <= n + 1; j++) {
            if (j != 0) cout << " ";
            if (j != 0 && j % 5 == 0) cout << endl;

            cout << A[i][j];
        }
        cout << endl;
    }
}