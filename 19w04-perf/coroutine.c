#include <stdbool.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


struct yield_state {
    jmp_buf buf;
    _Bool yielded;
};

#define yieldable(state)   struct yield_state * __state = state; \
                           if (__state->yielded) longjmp(__state->buf, 1); else {}

#define yield(x)  if (setjmp(__state->buf)) { __state->yielded = false;          }\
                  else                      { __state->yielded = true;  return x; }



long long busy_wait(int ref) {
    long long sum = 1;
    for (int x = 0; x < ref; x += 1) {
        sum = sum*x;
    }
    return sum;
}

int range(struct yield_state * state, int start, int end, int inc)
{
    yieldable(state);

    int i = 0;
    for (i = start; i < end - 1; i += inc) {
        busy_wait(0xffffff);
        yield(i);
    }
    return end - 1;
}

long long getsumA(int start, int end) {

    long long sum = 0;
    struct yield_state s;
    memset(&s, 0x0, sizeof(s));

    do {
        int x = range(&s, start, end, 1);
        sum += x;
    } while(s.yielded);

    return sum;
}

long long getsumB(int start, int end) {
    long long sum = 0;

    for (int x = start; x < end; x += 1) {
        busy_wait(0xffffff);
        sum += x;
    }
    return sum;
}

int main(void)
{

    long long sum = -1;
        
    sum = getsumB(0, 10);
    printf("%lld\n", sum);

    //sum = getsumA(0, 10);
    printf("%lld\n", sum);

    return EXIT_SUCCESS;
}

