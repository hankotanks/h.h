#define HH_IMPLEMENTATION
#include "hh.h"
#undef HH_IMPLEMENTATION

#include "test.h"

int
main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
    test_arr();
    test_path();
    return 0;
}
