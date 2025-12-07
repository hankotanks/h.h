#define HH_IMPLEMENTATION
#define HH_STRIP_PREFIXES
#include "hh.h"

int
main(int argc, char* argv[]) {
    (void) argc;
    char* path = path_alloc(argv[1]);
    MSG_BLOCK {
        printf("\n");
        printf("  path: %s\n", path);
        char* parent = path_parent(path_alloc(path));
        printf("  path_parent: %s\n", parent);
        path_free(parent);
        printf("  is_file: %s\n", STRINGIFY_BOOL(path_is_file(path)));
        printf("  is_root: %s\n", STRINGIFY_BOOL(path_is_root(path)));
    }
    path_free(path);
    return 0;
}
