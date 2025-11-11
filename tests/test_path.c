#include "test.h"

void
test_path(void) {
    MSG("Testing path implementation.");
    char* path = path_alloc(PROJECT_ROOT);
    arrfree(path);
}
