/* 
For all intents and purposes, we will create a 2D Mesh with
surface area of 1 and 8^2 nodes. The latter can be changed by
updating the parameter in the MESH2 declaration.
*/
ARCHITECTURE MESH2(8);

#include <math.h>

/* Written and refactored by Pride Yin */

/* Change m accordingly to the parameter of the architecture above */
#define m 8
#define points 50

int i;

float stream global[m * m];
float g[m * m];

float f(float t) {
    return(sqrt(4 - t * t));
}

void integrate(int idx, float *out) {
    float local = 0;
    float t;
    int j, k;
    float a, b, w;

    a = 0;
    b = 2;

    w = (b - a) / (m * m * points);

    t = a + idx * (b - a) / (m * m);

    for (j = 1; j <= points; j++) {  
        local = local + f(t);
        t = t + w;
    }
    local = w * local;

    float globalSum = local;

    /*
    We will begin by sending the bottom row of the mesh to the
    row above it.
    */
    if (idx >= ((m * m) - m) && idx <= m * m - 1) {
        send(global[idx - m], local);
    }

    /*
    This will wait and receive data to be sent from a node below it,
    then we will calculate a "global sum" of each partitioned row of
    the mesh.
    */
    if (idx <= ((m * m) - m) - 1) {
        recv(global[idx], globalSum);
    }
    globalSum = globalSum + local;

    /*
    We'll send the calculated data to the row above this process's row.
    Additionally, we will send the currently aggregated data once it
    reaches the second row of the mesh to the source. We will forcefully
    aggregate the data, simulating movement of data horizontally as compared
    to the vertical movement of data we've done previously when we're working
    with the first (source) row.
    */
    if (idx <= ((m * m) - m) - 1 && idx >= m) {
        send(global[idx - m], globalSum);
    } else if (idx >= 0 && idx < m) {
        send(global[0], globalSum);
    }

    *out = globalSum;
}

void aggregate() {
    /*
    This will aggregate the data from the first row of the mesh to the
    source node to calculate the integral (fixed by adding end points as well) of the
    function f(t) from a to b.
    */
    float agg, temp;

    while (global[0]??) {
        recv(global[0], temp);
        agg += temp;
    }
    float a, b, w;

    a = 0;
    b = 2;

    w = (b - a) / (m * m * points);

    cout << "For the function f(t): " << endl;
    cout << "    f(t) = sqrt(4 - t^2)" << endl;

    cout << "The integral of f(t) from 0 to 2 is: " << endl;
    cout << "   " << agg + w / 2 * (f(b) - f(a)) << endl;
}

main() {
    cout.precision(15);
    
    forall i = 0 to (m * m) - 1 @i do integrate(i, &g[i]);
    FORK aggregate(); JOIN;
}