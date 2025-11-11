#ifndef TEST_H__
#define TEST_H__

// a "better" approach is to supply these definitions
// as configuration options during the build process,
// but I've chosen to be explicit in this case
#define HH_LOG HH_LOG_DBG
#define HH_STRIP_PREFIXES
#include "hh.h"

void
test_path(void);
void
test_arr(void);

#endif // TEST_H__
