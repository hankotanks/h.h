#define HH_IMPLEMENTATION
#include "h.h"

#define FIBONACCI_MAX 1000000

int
main(void) {
    PROFILE(root);
    MSG_BLOCK
    for(size_t fib[2] = {0, 1}; fib[0] < FIBONACCI_MAX; 
        memswap((char*) fib, (char*) &fib[1], (char*) &fib[1], (char*) &fib[2]), 
        fib[0] += fib[1]) {
        PROFILE(add, root);
        size_t sum = 0;
        for(size_t i = 0; i <= fib[0]; sum += (i++)) {
            PROFILE(inner, add);
            PROFILE_END(inner);
        }
        PROFILE(second, add);
        PROFILE_END(second);
        PROFILE_END(add);
        LOG_APPEND("%zu [%zu], ", fib[0], sum);
    }
    PROFILE_END(root);
    return 0;
}
