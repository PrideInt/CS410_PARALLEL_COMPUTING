#include <stdlib.h>  
#include <math.h>

/* Written by Pride Yin */

#define n 100
#define m 100
#define bucket_number 10

int data[n + 1], sorted[n + 1];
int bucket[bucket_number][n + 1];
int sums[bucket_number][1];
spinlock l[n + 1];
int stream s[bucket_number + 1];

int i, j;

void sort(int d[]) {
    int j, k, max, max_index, temp;

    for (j = d[0]; j > 1; j--) {
        max = d[j];
        max_index = j;

        for (k = 1; k < j; k++) {
            if (d[k] > max) {
                max = d[k];
                max_index = k;
            }
        }
        temp = d[j];
        d[j] = max;
        d[max_index] = temp;		
    }
    return;
}

main() {
    for (i = 1; i <= n; i++) {
        data[i] = rand() % m;
    }

    /* SEQUENTIAL:

    int v, b_i, index;
    
    for (i = 1; i <= n; i++) {
        v = data[i];
        b_i = (int)(v/10.0);
        cout << v << ": " << b_i << "; " << endl;
        index = ++bucket[b_i][0];
        bucket[b_i][index] = v;
    }
    
    for (i = 0; i <= 9; i++) {
        sort(bucket[i]);
    }

    int k = 1;
    for (i = 0; i <= 9; i++) {
        for (j = 1; j <= bucket[i][0]; j++) {
            sortedData[k++] = bucket[i][j];
        }
    }
    
    k--;
    for (i = 1; i <= k; i++) {
        cout << "d[" << i << "]=" << sortedData[i] << endl;
    }
    */

    /* PARALLEL: */
    forall i = 1 to n grouping bucket_number do {
        int v, buck, index;
        
        v = data[i];
        buck = (int) (v / 10.0);

        /* cout << v << ": " << buck << "; " << endl; */

        Lock(l[buck]);

        index = ++bucket[buck][0];
        bucket[buck][index] = v;

        Unlock(l[buck]);
    }

    forall i = 1 to bucket_number do {
        sort(bucket[i - 1]);
    }

    /* VERIFY SORTED NUMBERS:

    for (i = 1; i <= bucket_number; i++) {
        cout << "Bucket " << i << endl;
        for (j = 1; j <= bucket[i - 1][0]; j++) {
            cout << "bucket[" << i << "][" << j << "]=" << bucket[i - 1][j] << endl;
        }
    }
    */

    forall i = 1 to bucket_number do {
        int start, k;
        int p;

        if (i > 1) {
            recv(s[i], start);

            if (i < bucket_number) {
                send(s[i + 1], start + bucket[i - 1][0]);
            }
        } else {
            start = 0;
            send(s[i + 1], bucket[i - 1][0]);
        }
        k = start;

        for (p = 1; p <= bucket[i - 1][0]; p++) {
            int val = bucket[i - 1][p];

            sorted[k] = val;
            k++;
        }
    }

    for (i = 0; i < n; i++) {
        cout << "d[" << i + 1 << "]=" << sorted[i] << endl;
    }
}