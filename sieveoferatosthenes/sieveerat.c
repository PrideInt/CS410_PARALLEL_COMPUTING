#include <stdlib.h>
#include <math.h>

/* Written by Pride Yin */

/* Variables and definitions for the sequential program and subsequent parallelized program */
#define n 499

boolean prime[n + 1];
int i, num, loc;

/* Variables and definitions for the parallelized program */
#define partition 10

int parts = n / partition;
int j;

int stream portions[partition + 2];
int stream ports[partition + 2];
int stream operations;

void broadcast() {
    int start = 0;

    int ops;

    for (num = 2; num <= sqrt(n); num++) {
        if (prime[num]) {
            ops++;
            send(portions[start + 1], num);
        }
    }
    send(operations, ops);
}

void communicate(int i) {
    int step;

    while (true) {
        if (portions[i]??) {
            recv(portions[i], step);
            send(ports[i], step);

            send(portions[i + 1], step);
        }
    }
}

void calculate() {
    int step;

    int o, ops;

    while (true) {
        for (j = 1; j <= partition; j++) {
            recv(ports[j], step);

            ops++;

            int first = ((n / partition) * (j - 1));
            if (n % partition != 0) {
                first = ((n / partition + 1) * (j - 1));
            }
            if (first < partition) {
                first = step * 2;
            }
            int bound = first + parts;

            int r = first % step;
            if (r != 0) {
                first = (first / step + 1) * step;
            }

            for (loc = first; loc <= bound; loc += step) {
                if (loc <= n) {
                    prime[loc] = false;
                }
            }
        }
        if (operations??) {
            recv(operations, o);
        }

        if (ops == o * partition) {
            for (i = 1; i <= n; i++) {
                cout << prime[i] << " ";

                if (i % 10 == 0) {
                    cout << endl;
                }
            }
            exit(1);
        }
    }
}

main() {
    /* Sequential:
    for (i = 1; i <= n; i++) {
        prime[i] = true;
    }

    for (num = 2; num <= sqrt(n); num++) {
        if (prime[num]) {
            loc = num + num;
        
            while (loc <= n) {
                prime[loc] = false;
                loc = loc + num;
            }
        }
    }

    for (i = 1; i <= n; i++) {
        cout << prime[i] << " ";

        if (i % 10 == 0) {
            cout << endl;
        }
    }
    */
    
    /* Parallel: */
    forall i = 0 to n grouping partition do prime[i] = true;

    FORK broadcast();

    FORK calculate();

    forall i = 1 to partition do communicate(i);
}