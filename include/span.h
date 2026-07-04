#ifndef HH_SPAN__
#define HH_SPAN__

#include "core.h"

// SECTION(HEADER)
// hh_span_t is a string-view interface
// intended for parsing
typedef struct {
    char* ptr;
    char* end;
} hh_span_t;

// options for the hh_span_next family of functions/macros
// delim: the separator sequence used to split tokens
// delim_as_set: treat `delim` as a set of possible delimiters rather than a sequence
// eol: treat '\n' as a valid delimiter
// trim: trim whitespace around tokens
typedef struct {
    const char* delim;
    _Bool delim_as_set;
    _Bool eol;
    _Bool trim;
} hh_span_opt;

// returns the length of the span
#define hh_span_len(span) (((span).ptr != NULL && (span).end != NULL) ? ((size_t) ((span).end - (span).ptr)) : 0)

// format specifier and arg macro for span's
// printf(hh_span_fmt "\n", hh_span_fmt_args(span));
#define hh_span_fmt "%.*s"
#define hh_span_fmt_args(span) ((int) hh_span_len(span)), ((span).ptr)

// TODO: consider implementing a const version of hh_span_t
// creates a stack-allocated span from a null-terminated cstr
// the span does NOT contain the null-terminator
hh_span_t
hh_span(char* contents);

// grabs the next token from the span
#define hh_span_next(span, ...) hh_span_next_opt((span), (hh_span_opt) { __VA_ARGS__ })
// SECTION(HEADER, END)

//
//
//

//
//
//

//
//
//

//
//
//

// SECTION(HEADER_PRIVATE)
hh_span_t
hh_span_next_opt(hh_span_t* s, hh_span_opt opt);
// SECTION(HEADER_PRIVATE, END)

#ifdef HH_IMPLEMENTATION
// SECTION(IMPLEMENTATION)

hh_span_t
hh_span(char* str) {
    if(str == NULL) return (hh_span_t) {0};
    return (hh_span_t) { .ptr = str, .end = str + strlen(str) };
}

size_t
HH__span_matches(hh_span_t* span, hh_span_opt opt) {
    if(span->ptr == span->end) return SIZE_MAX;
    if(opt.eol && span->ptr[0] == '\n') return 1;
    if(opt.delim == NULL) return 0;
    if(opt.delim_as_set) {
        return (strchr(opt.delim, span->ptr[0]) != 0);
    } else {
        size_t count;
        count = strlen(opt.delim);
        if(span->ptr + count >= span->end) return 0;
        return (strncmp(span->ptr, opt.delim, count) == 0) ? count : 0;
    }
}

hh_span_t
hh_span_next_opt(hh_span_t* span, hh_span_opt opt) {
    const char* whitespace = opt.eol ? " \t\r" : " \t\r\n";
    hh_span_t temp = { .end = span->end };
    if(span->ptr == span->end) return temp;
    if(opt.trim) {
        while(strchr(whitespace, span->ptr[0]) != 0) ++(span->ptr);
    }
    size_t count;
    for(char* cur = span->ptr, *adv; cur <= span->end; ++cur) {
        temp.ptr = cur;
        count = HH__span_matches(&temp, opt);
        if(count == SIZE_MAX) {
            if(opt.trim) {
                --cur;
                while(cur > span->ptr && strchr(whitespace, cur[0]) != 0) --cur;
                ++cur;
            }
            temp.ptr = span->ptr;
            temp.end = cur;
            span->ptr = span->end;
            return temp;
        }
        if(count > 0) {
            adv = cur + count;
            if(opt.trim) {
                --cur;
                while(cur > span->ptr && strchr(whitespace, cur[0]) != 0) --cur;
                ++cur;
                while(adv < span->end && strchr(whitespace, adv[0]) != 0) ++adv;
            }
            temp.ptr = span->ptr;
            temp.end = cur;
            span->ptr = adv;
            return temp;
        }
    }
    temp.ptr = NULL;
    return temp;
}
// SECTION(IMPLEMENTATION, END)
#endif // HH_IMPLEMENTATION
#endif // HH_SPAN__

#ifndef HH__APPLY_PREFIXES
#define HH__APPLY_PREFIXES
#ifndef HH_APPLY_PREFIXES
// SECTION(PREFIX)
#define span_t hh_span_t
#define span_opt hh_span_opt
#define span_len hh_span_len
#define span_fmt hh_span_fmt
#define span_fmt_args hh_span_fmt_args
#define span hh_span
#define span_next hh_span_next
// SECTION(PREFIX, END)
#endif // HH_APPLY_PREFIXES
#endif // not HH__APPLY_PREFIXES
