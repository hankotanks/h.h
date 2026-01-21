#define HH_IMPLEMENTATION
#define HH_STRIP_PREFIXES
#include "h.h"

int
main(void) {
    // create a new path
    char* path_root = path_alloc(PROJECT_ROOT);
    DBG("Created path_root: "
        "path = \"%s\", len = %zu, cap = %zu", path_root, darrlen(path_root), darrcap(path_root));
    // check joining
    const char* name_header = "hh.h";
    char* path_header = path_alloc(path_root);
    path_join(path_header, name_header);
    ASSERT(path_header != NULL, "hh_path_join returned NULL");
    ASSERT(strcmp(path_name(path_header), name_header) == 0, 
        "hh_path_name returned incorrect filename: "
        "path = %s, len = %zu, cap = %zu", path_header, darrlen(path_header), darrcap(path_header));
    DBG("Joined \"hh.h\" to path_root: "
        "path = %s, len = %zu, cap = %zu", path_header, darrlen(path_header), darrcap(path_header));
    // take parent
    hh_path_parent(path_header);
    ASSERT(path_header != NULL, "hh_path_parent returned NULL before system root");
    DBG("Took parent of path_header: "
        "path = %s, len = %zu, cap = %zu", path_header, darrlen(path_header), darrcap(path_header));
    // assert that adding and removing path components returns identical strings
    ASSERT(strcmp(path_root, path_header) == 0, 
        "hh_path_parent returned unexpected result: "
        "path = %s, len = %zu, cap = %zu", path_header, darrlen(path_header), darrcap(path_header));
    // free path_header
    path_free(path_header);
    ASSERT(path_header == NULL, 
        "hh_path_free (hh_darrfree) did not set NULL after free: "
        "path = %s, len = %zu, cap = %zu",  path_header, darrlen(path_header), darrcap(path_header));
    // descend from project root to system root
    DBG("Traversing from path_root to system root: "
        "path = %s, len = %zu, cap = %zu", path_root, darrlen(path_root), darrcap(path_root));
    while(!path_is_root(path_root)) {
        ASSERT(path_exists(path_parent(path_root)), 
            "hh_path_parent returned non-existent path while not at system root: "
            "path = %s, len = %zu, cap = %zu",  path_root, darrlen(path_root), darrcap(path_root));
        ASSERT(strlen(path_root) + 1 == darrlen(path_root), 
            "hh_path_parent broke parity between hh_darrlen and strlen: "
            "path = %s, len = %zu, cap = %zu",  path_root, darrlen(path_root), darrcap(path_root));
        DBG("Took parent of path_root: "
            "path = %s, len = %zu, cap = %zu", path_root, darrlen(path_root), darrcap(path_root));
    }
    // validate system root properties
    ASSERT(path_parent(path_root) == NULL, 
        "hh_path_parent returned non-NULL while hh_path_is_root was truthy: "
        "path = %s, len = %zu, cap = %zu",  path_root, darrlen(path_root), darrcap(path_root));
    ASSERT(path_exists(path_root), 
        "hh_path_exists was false while hh_path_is_root was truthy: "
        "path = %s, len = %zu, cap = %zu",  path_root, darrlen(path_root), darrcap(path_root));
    ASSERT(!path_is_file(path_root), 
        "hh_path_is_file reported that system root is a file, not a directory: "
        "path = %s, len = %zu, cap = %zu",  path_root, darrlen(path_root), darrcap(path_root));
    // free path_root
    path_free(path_root);
    ASSERT(path_root == NULL, "hh_path_free (hh_darrfree) did not set NULL after free");
    return 0;
}
