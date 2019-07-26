#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

void matrix_init_random(int x, int m[x][x])
{
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < x; j++) {
            m[i][j] = rand()/0xffffff;
        }
    }
}

void matrix_dump(int x, int m[x][x])
{
    printf("[\n");
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < x; j++) {
            printf("%d ", m[i][j]);
        }
        printf(";\n");
    }
    printf("]\n");

}

void sqmat_mult(int x, const int a[x][x], const int b[x][x], int m[x][x])
{
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < x; j++) {
            int sum = 0;
            for (int k = 0; k < x; k++) {
                sum += a[i][k] * b[k][j];
            }
            m[i][j] = sum;
        }
    }
}


void sqmat_transpose(int x, int a[x][x])
{
    for (int i = 0; i < x; i++) {
        for (int j = i+1; j < x; j++) {
            int temp = a[i][j];
            a[i][j] = a[j][i];
            a[j][i] = temp;
        }
    }     
}

void sqmat_mult_efficient(int x, const int a[x][x], int b[x][x], int m[x][x])
{
    sqmat_transpose(x, b);

    for (int i = 0; i < x; i++) {
        for (int j = 0; j < x; j++) {
            int sum = 0;
            for (int k = 0; k < x; k++) {
                sum += a[i][k] * b[j][k];
            }
            m[i][j] = sum;
        }
    }

    sqmat_transpose(x, b);
}


int main(void)
{
    int p = 128; // an O(1k**3) takes a lot of time even on mordern processors
    srand(13); // random seed for predictable outcome

    struct sched_param schedp;
    schedp.sched_priority = 1;
    sched_setscheduler(0, SCHED_FIFO, &schedp);

    // Allocate memory for the matrices and intialise them
    void* first = malloc(sizeof(int) * p * p);
    void* second = malloc(sizeof(int) * p * p);
    void* multiply = malloc(sizeof(int) * p * p);
    if (first == NULL || second == NULL || multiply == NULL) {
        return EXIT_FAILURE;
    }
    matrix_init_random(p , first);
    matrix_init_random(p , second);

    sqmat_mult(p, first,  second, multiply);

    return EXIT_SUCCESS;
}

