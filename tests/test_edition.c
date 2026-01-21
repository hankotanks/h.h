#define HH_IMPLEMENTATION
#define HH_STRIP_PREFIXES
#include "h.h"

int
main(void) {
    DBG("C89 support: %s", STRINGIFY_BOOL(edition_supported(EDITION_89)));
    DBG("C90 support: %s", STRINGIFY_BOOL(edition_supported(EDITION_90)));
    DBG("C94 support: %s", STRINGIFY_BOOL(edition_supported(EDITION_94)));
    DBG("C99 support: %s", STRINGIFY_BOOL(edition_supported(EDITION_99)));
    DBG("C11 support: %s", STRINGIFY_BOOL(edition_supported(EDITION_11)));
    DBG("C17 support: %s", STRINGIFY_BOOL(edition_supported(EDITION_17)));
    DBG("C23 support: %s", STRINGIFY_BOOL(edition_supported(EDITION_23)));
}