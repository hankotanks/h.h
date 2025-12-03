#define HH_IMPLEMENTATION
#define HH_STRIP_PREFIXES
#include "hh.h"

#include <stdint.h>

void
test_darr_print(char** arr) {
    printf("\"");
    for(size_t i = 0; i < darrlen(arr); ++i) printf("%s ", arr[i]);
    printf("\"\n");
}

int
main(void) {
    char** arr = NULL, fst[6], snd[6];
    darrput(arr, "Hello");
    darrput(arr, "World");
    darrput(arr, "This");
    darrput(arr, "is");
    darrput(arr, "a");
    darrput(arr, "test");
    DBG_BLOCK {
        printf("Original: arr = ");
        test_darr_print(arr);
    }
    // randomly swap a few times
    size_t i, j, k;
    for(i = 0; i < darrlen(arr); ++i) {
        j = (size_t) ((uint64_t) rand() * darrlen(arr) / (RAND_MAX + 1ULL));
        k = (size_t) ((uint64_t) rand() * darrlen(arr) / (RAND_MAX + 1ULL));
        strcpy(fst, arr[j]);
        strcpy(snd, arr[k]);
        darrswap(arr, j, k);
        DBG_BLOCK {
            printf("Swapped \"%s\" and \"%s\": arr = ", fst, snd);
            test_darr_print(arr);
        }
        ASSERT(strcmp(fst, arr[k]) == 0 && strcmp(snd, arr[j]) == 0, 
            "hh_darrswap failed to swap elements");
    }
    // pop swap
    j = darrlen(arr);
    for(i = 0; i < j; ++i) {
        k = (size_t) ((uint64_t) rand() * darrlen(arr) / (RAND_MAX + 1ULL));
        char* tmp = darrswapdel(arr, k);
        DBG_BLOCK {
            printf("Removed \"%s\": len = %zu, idx = %zu, arr = ", tmp, darrlen(arr), k);
            test_darr_print(arr);
        }
        ASSERT(darrlen(arr) + 1 == (j - i), 
            "hh_darrswapdel failed to remove element: len = %zu, expected = %zu", darrlen(arr), j - i - 1);
    }
    ASSERT(darrlen(arr) == 0, 
        "hh_darrswapdel failed to remove all elements");
    ASSERT(darrswapdel(arr, 0) == NULL, 
        "hh_darrswapdel did not return NULL on empty array");
    darrfree(arr);
    return 0;
}
