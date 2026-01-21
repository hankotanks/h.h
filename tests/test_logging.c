#define HH_IMPLEMENTATION
#define HH_STRIP_PREFIXES
#include "hh.h"

int main(void) {
    int test[10];
    for(size_t i = 0; i < ARR_LEN(test); ++i) test[i] = (int) i;
    DBG_BLOCK {
        LOG_APPEND("[");
        for(size_t i = 0; i < ARR_LEN(test); ++i) {
            LOG_APPEND(" %d", test[i]);
            if(i + 1 != ARR_LEN(test)) LOG_APPEND(",");
        }
        LOG_APPEND(" ]");
    }
    return 0;
}
