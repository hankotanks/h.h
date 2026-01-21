#define HH_IMPLEMENTATION
#define HH_STRIP_PREFIXES
#include "h.h"

int
main(void) {
    const char text[] = \
        "Hello, 3.14159 World!\n";
    size_t token_index = 0;
    span_t span;
    ASSERT(span_init(&span, text, ","), "Failed to initialize span");
    char hello[32];
    ASSERT(span_parse(&span, "%s", hello), "Failed to parse token [%zu]", token_index);
    ASSERT(strlen(hello) == span.len, 
        "Mismatch between parsed string's length and the span's length: "
        "len = %zu, expected = %zu", strlen(hello), span.len);
    DBG("Token [%zu]: len = %zu, token = \"%s\", skips = %zu", token_index++, span.len, hello, span.skips);
    span.delim = NULL;
    ASSERT(span_next(&span), "Failed to parse token [%zu]", token_index);
    double pi;
    ASSERT(span_parse(&span, "%lf", &pi), "Failed to parse token [%zu]", token_index);
    DBG("Token [%zu]: len = %zu, token = %lf", token_index++, span.len, pi);
    ASSERT(span_next(&span), "Failed to parse token [%zu]", token_index);
    char world[32];
    ASSERT(span_parse(&span, "%s", world), "Failed to parse token [%zu]", token_index);
    ASSERT(strlen(world) == span.len, 
        "Mismatch between parsed string's length and the span's length: "
        "len = %zu, expected = %zu", strlen(world), span.len);
    DBG("Token [%zu]: len = %zu, token = \"%s\"", token_index++, span.len, world);
    (void) token_index;
    return 0;
}
