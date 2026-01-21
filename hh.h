#ifndef HH__
#define HH__

#ifndef _WIN32
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#else
#ifdef __MINGW32__
#ifdef _MSC_VER
#define __USE_MINGW_ANSI_STDIO
#endif // _MSC_VER
#endif // __MINGW32__
#endif // _WIN32

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

// log only errors
// #define HH_LOG HH_LOG_ERR
// log errors and messages
// #define HH_LOG HH_LOG_MSG
// log everything
// #define HH_LOG HH_LOG_DBG
#define HH_LOG_ERR 0
#define HH_LOG_MSG 1
#define HH_LOG_DBG 2

#ifndef HH_ERR_STREAM
#define HH_ERR_STREAM stderr
#endif // HH_ERR_STREAM
#ifndef HH_MSG_STREAM
#define HH_MSG_STREAM stdout
#endif // HH_MSG_STREAM
#ifndef HH_DBG_STREAM
#define HH_DBG_STREAM stdout
#endif // HH_DBG_STREAM

// all logging functions have the same behavior as printf,
// HH_ERR logs to stderr instead of stdout
#ifdef HH_LOG
#if HH_LOG >= HH_LOG_DBG
#define HH_DBG(...) do { \
    fprintf(HH_DBG_STREAM, "DEBUG [%s:%d]: ", __FILE__, __LINE__); \
    fprintf(HH_DBG_STREAM, __VA_ARGS__); \
    fputc('\n', HH_DBG_STREAM); \
} while(0)
#else
#define HH_DBG(...)
#endif // HH_DBG
#if HH_LOG >= HH_LOG_MSG
#define HH_MSG(...) do { \
    fprintf(HH_MSG_STREAM, "INFO [%s:%d]: ", __FILE__, __LINE__); \
    fprintf(HH_MSG_STREAM, __VA_ARGS__); \
    fputc('\n', HH_MSG_STREAM); \
} while(0)
#else
#define HH_MSG(...)
#endif // HH_MSG
#if HH_LOG >= HH_LOG_ERR
#define HH_ERR(...) do { \
    fprintf(HH_ERR_STREAM, "ERROR [%s:%d]: ", __FILE__, __LINE__); \
    fprintf(HH_ERR_STREAM, __VA_ARGS__); \
    fputc('\n', HH_ERR_STREAM); \
} while(0)
#else
#define HH_ERR(...)
#endif // HH_ERR
#else
#define HH_DBG(...)
#define HH_MSG(...)
#define HH_ERR(...)
#endif // HH_LOG

// the block logging functions allow building a log message incrementally,
// useful for printing arrays, etc.
// within the block, HH_LOG_APPEND extends the log
// the log statement produced is automatically newline-terminated 
#ifdef HH_LOG
#if HH_LOG >= HH_LOG_DBG
#define HH_DBG_BLOCK HH_LOG_BLOCK(HH_DBG_STREAM, "DEBUG")
#else
#define HH_DBG_BLOCK if(0)
#endif // HH_DBG
#if HH_LOG >= HH_LOG_MSG
#define HH_MSG_BLOCK HH_LOG_BLOCK(HH_MSG_STREAM, "INFO")
#else
#define HH_MSG_BLOCK if(0)
#endif // HH_MSG
#if HH_LOG >= HH_LOG_ERR
#define HH_ERR_BLOCK HH_LOG_BLOCK(HH_ERR_STREAM, "ERROR")
#else
#define HH_ERR_BLOCK if(0)
#endif // HH_ERR
#define HH_LOG_APPEND(...) fprintf((FILE*) HH_LOG_BLOCK_stream, __VA_ARGS__)
#else
#define HH_DBG_BLOCK if(0)
#define HH_MSG_BLOCK if(0)
#define HH_ERR_BLOCK if(0)
#define HH_LOG_APPEND(...)
#undef HH_LOG_BLOCK
#endif // HH_LOG

// min and max
#define HH_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define HH_MIN(x, y) (((x) < (y)) ? (x) : (y))

// array length (ONLY for stack-allocated arrays)
#define HH_ARR_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

// useful attributes for function declarations
#if defined(__GNUC__) || defined(__clang__)
#define HH_UNUSED __attribute__((unused))
#else
#define HH_UNUSED
#endif
#if defined(__GNUC__) || defined(__clang__)
#define HH_FALLTHROUGH __attribute__((fallthrough))
#else
#define HH_FALLTHROUGH
#endif

// stringify
#define HH_STRINGIFY(x) HH_STRINGIFY_HELPER(x)
// stringify booleans
// example: printf("flag: %s\n", HH_STRINGIFY_BOOL(flag));
#define HH_STRINGIFY_BOOL(x) ((x) ? "true" : "false")

// standard assertion with variadic error message
#define HH_ASSERT(cond, ...) do { if(cond) break; HH_ERR(__VA_ARGS__); assert(cond); } while(0)
#define HH_ASSERT_UNREACHABLE(cond) HH_ASSERT(cond, "Unreachable!")
// this is an assertion that will execute the statement 
// or block that follows it before exiting
#define HH_ASSERT_BEFORE(cond) for(; !(cond); assert(cond))
// use this for unreachable branches
// for example, default cases in switch blocks
#define HH_UNREACHABLE HH_ASSERT_UNREACHABLE(0)

// wrappers that assert allocation success
void*
hh_malloc_checked(size_t size);
void*
hh_calloc_checked(size_t num, size_t size);

// union to easily pass around and store function pointers as data pointers
// without breaking C99 conventions
typedef union {
    const void* _ptr;
    void (*_fp)(void);
} hh_fp_wrap_t;

// accepts a function pointer and returns hh_fp_wrap_t
#define hh_fp_wrap(fp) ((hh_fp_wrap_t) { ._fp = (void(*)(void)) (fp) })
// accepts hh_fp_wrap_t* and returns a function pointer specified by fp_type
#define hh_fp_unwrap(fp_wrap, fp_type) ((fp_type) (((hh_fp_wrap_t*) (fp_wrap))->_fp))

// Adapted from...
// stb_ds.h - v0.67 - public domain data structures - Sean Barrett 2019

#define hh_darrclear(arr)      ((arr == NULL) ? 0 : (hh_darrheader(arr)->len = 0))
#define hh_darrfree(arr)       ((void) ((arr) ? free(hh_darrheader(arr)) : (void) 0), (arr) = NULL)
#define hh_darrlast(arr)       ((arr)[hh_darrheader(arr)->len - 1])
#define hh_darrput(arr, val)   ((void) hh_darrgrow(arr, 1), (arr)[(hh_darrheader(arr)->len)++] = (val))
#define hh_darrpop(arr)        ((arr)[--(hh_darrheader(arr)->len)])
#define hh_darradd(arr, n)     (HH__darradd((void**) &(arr), (n), sizeof *(arr)))
#define hh_darrlen(arr)        ((arr == NULL) ? 0 : hh_darrheader(arr)->len)
#define hh_darrsetlen(arr, n)  ((arr == NULL) ? 0 : (hh_darrheader(arr)->len = (n)))
#define hh_darrcap(arr)        ((arr == NULL) ? 0 : hh_darrheader(arr)->cap)
// returns truthy if swap succeeded
// swap fails on empty dynamic arrays
#define hh_darrswap(arr, i, j) (HH__darrswap((arr), (i), (j)))
#define hh_darrswapdel(arr, i) ((i) < hh_darrlen(arr) ? (HH__darrswap((arr), (i), hh_darrlen(arr) - 1), hh_darrpop(arr)) : NULL)
 
// append a string to a dynamic array
// ensures null-termination
#define hh_darrputstr(arr, str) do { \
        if(hh_darrlen(arr) == 0 || (hh_darrlen(arr) != 0 && hh_darrlast(arr) != '\0')) hh_darrput(arr, '\0'); \
        assert(hh_darrlast(arr) == '\0'); \
        size_t _tmp = hh_darradd(arr, strlen(str)) - 1; \
        strcpy((arr) + _tmp, (str)); \
    } while(0)

// type representing a memory arena
typedef struct HH__arena hh_arena;

// allocates memory within an arena
// assumes 0-initialization
// any size is valid, even if it is >= HH_ARENA_DEFAULT_SIZE
void*
hh_arena_alloc(hh_arena* arena, size_t sz);
// free the given memory arena
// does not free(arena), it must be freed separately if it was heap-allocated
void
hh_arena_free(hh_arena* arena);

// hh_path_alloc
// [in const] raw: a cstr representing a raw path
// return: heap-allocated dynamic array containing the normalized path
// This function creates a normalized path where...
// * relative paths are made absolute
// * backslashes "\\" are converted to forward slashes "/"
// * (WINDOWS ONLY) volume names are capitalized "c:" -> "C:"
// * final slashes are stripped
char*
hh_path_alloc(const char* raw);
// hh_path_exists
// [in const] path: a path originally constructed with hh_path_alloc
// return: truthy if path exists, false otherwise
_Bool
hh_path_exists(const char* path);
// hh_path_is_file
// [in const] path: a path originally constructed with hh_path_alloc
// return: truthy if path exists and points to a file, false otherwise
_Bool
hh_path_is_file(const char* path);
// hh_path_is_root
// [in const] path: a path originally constructed with hh_path_alloc
// return: truthy if path is a root path (eg. "C:/" or "/"), false otherwise
_Bool 
hh_path_is_root(const char* path);
// hh_path_join
// [in] path: a path originally constructed with hh_path_alloc
// [in] ...: any number of path elements that should be joined
// return: same as [in] path
// This is structued as a macro to avoid the case where a reallocation
// causes the input and output pointers to differ.
// NULL is returned when the provided path is NULL
#define hh_path_join(path, ...) ((path) = HH__path_join((path), __VA_ARGS__, NULL))
// hh_path_name
// [in const] path: a path originally constructed with hh_path_alloc
// return: a pointer to the first character of the last path element
// This function does not allocate
const char*
hh_path_name(const char* path);
// hh_path_parent
// [in] path: a path originally constructed with hh_path_alloc
// return: same as [in] path
// Removes the final element from the path
// The return pointer is always equal to the given path
char*
hh_path_parent(char* path);
// hh_path_free
// [in] path: a path originally constructed with hh_path_alloc
// return: void
// Frees the path and sets it to NULL
#define hh_path_free hh_darrfree

// each value represents a major release of the C standard
// this allows you to check the standard at both compile and runtime
#define HH_EDITION_89 0L
#define HH_EDITION_90 1L
#define HH_EDITION_94 199409L
#define HH_EDITION_99 199901L
#define HH_EDITION_11 201112L
#define HH_EDITION_17 201710L
#define HH_EDITION_23 202311L

typedef long hh_edition_t;

// hh_edition_supported
// [in] ed: the standard you want to check (HH_EDITION_89, etc)
// return: truthy if the standard is supported, false otherwise
_Bool
hh_edition_supported(hh_edition_t ed);

// compile time checking, with identical logic to hh_edition_supported above
#define HH_EDITION_SUPPORTED(ed) (HH_EDITION >= (ed))

// represents a non-owning view into a char buffer
typedef struct {
    const char* ptr;
    const char* delim;
    size_t len;
    size_t skips;
} hh_span_t;

// initialize a span and advance to the first token
// NOTE: the delimiter can be changed at any point without causing issues
_Bool
hh_span_init(hh_span_t* span, const char* ptr, const char* delim);
// advances the span to the start of the next token
// returns truthy unless the end of the buffer has been reached 
// and no more tokens remain
// delim represents the expected dividers between tokens (excluding whitespace)
_Bool
hh_span_next(hh_span_t* span);
// advance to the next line, skipping all remaining tokens
_Bool
hh_span_next_line(hh_span_t* span);
// returns true if the span's current token equals `other`
_Bool
hh_span_equals(const hh_span_t span, const char* other);
// parse a given type from the span
// does not advance to the next token
_Bool
hh_span_parse(const hh_span_t* span, const char* fmt, void* out);
// same as above, but advances the span ot the next token
// so hh_span_parse_next can be called in succession
_Bool
hh_span_parse_next(hh_span_t* span, const char* fmt, void* out);

// templates for custom key hashing and comparator functions
typedef size_t (*hh_map_hash_f)(const void* key, size_t size_key);
// hh_map_comp_f's return value follows the same paradigm as memcmp or strcmp
//  0 indicates equality
// -1 means key_a is lexographically less than key_b
//  1 indicates it is greater than
typedef int (*hh_map_comp_f)(const void* key_query, size_t size_key_query, const void* key_in, size_t size_key_in);
// signature for freeing both keys and values
typedef void (*hh_map_free_f)(const void* ptr, size_t size_ptr);

// hashmap data structure with variably-sized key-value pairs
// on initialization, only the bucket_count must be provided
// if hash and comp functions are not given, defaults are used
// standard initialization:
// hh_map_t* hm = { .bucket_count = 32, 0 };
// NOTE: all other fields should be 0-initialized
typedef struct {
    size_t bucket_count;
    hh_map_hash_f hash;
    hh_map_comp_f comp;
    hh_map_free_f free_key;
    hh_map_free_f free_val;
    char** buckets;
} hh_map_t;

// represents an element returned by hh_map_get
// changing the data pointer to by `val` is UB
// unless the length is preserved
typedef struct {
    size_t size_key, size_val;
    const void* key;
    const void* val;
} hh_map_entry_t;

// insert a key-value pair into the hashmap
_Bool
hh_map_insert(hh_map_t* map, const void* key, size_t size_key, const void* val, size_t size_val);
// macro for inserting string keys
// NOTE: the key stored in the hashmap is null-terminated,
// which means hh_map_get calls MUST also include a null-terminator with default hh_map_comp_f behavior
#define hh_map_insert_with_cstr_key(map, key, val, size_val) hh_map_insert(map, key, strlen(key), val, size_val)
// insert an hh_map_entry_t element
// useful for copying from one hashmap to another
_Bool
hh_map_insert_entry(hh_map_t* map, const hh_map_entry_t* entry);
// returns the key-value pair associated with a given key
// if the key does not exist in the map, the entry is 0-initialized
// NOTE: changing the underlying key & value data is a corrupting action
// if the length overruns size_key or size_val, respectively
hh_map_entry_t
hh_map_get(const hh_map_t* map, const void* key, size_t size_key);
// macro for querying with cstr keys
#define hh_map_get_with_cstr_key(map, key) hh_map_get(map, key, strlen(key))
// returns the value corresponding to the given key
// NULL if the key is not a member of the map
const void*
hh_map_get_val(const hh_map_t* map, const void* key, size_t size_key);
// another helper that returns the value pointer for a cstr key, instead of the entry
#define hh_map_get_val_with_cstr_key(map, key) hh_map_get_val(map, key, strlen(key))
// remove entry corresponding to the given key
// returns truthy if an entry was removed
_Bool
hh_map_remove(hh_map_t* map, const void* key, size_t size_key);
// iterator macro for hh_map
// hh_map_it(&map, it) printf("%.*s", (int) it.size_key, it.key);
#define hh_map_it(map, it) for(hh_map_entry_t it = HH__map_it_begin(map); it.val; HH__map_it_next(map, &it))
// free hh_map_t
void
hh_map_free(hh_map_t* map);

// structure representing the argument parser tree
// NOTE: must be 0 initialized
// hh_args_t manages all allocations internally, including parsed paths
typedef struct HH__args_t hh_args_t;

// the types of values that can be parsed
// hh_args_add_flag's return value can be directly assigned to the types
// shown to the right of the enumerators
typedef enum {
    HH_FLAG_BOOL, // const _Bool*
    HH_FLAG_CSTR, // char* const*
    HH_FLAG_PATH, // char* const*
    HH_FLAG_DBL,  // const double*
    HH_FLAG_LONG, // const long*
    HH_FLAG_ULONG // const unsigned long*
} hh_flag_type;

// configuration options for hh_args_add_flag
// all fields are optional,
// except for flag, flag_long (one of which must be set)
typedef struct {
    char flag;
    const char* flag_long;
    const char* name;
    const char* desc;
    _Bool required;
} hh_flag_opt;

// add a flag to the argument parser
#define hh_args_add_flag(args, type, ...) HH__args_add_flag((args), (type), (hh_flag_opt) { __VA_ARGS__ })
// add a new command to the argument parser
hh_args_t*
hh_args_add_cmd(hh_args_t* args, const char* name, const char* desc);
// parse command line arguments
// returns truthy on success
_Bool
hh_args_parse(hh_args_t* args, FILE* stream, int argc, char* argv[]);
// free the argument parser tree
// NOTE: only needs to be invoked on the root node of the tree
void
hh_args_free(hh_args_t* args);
// write parsing error to stream (if one occurred)
void
hh_args_print_error(const hh_args_t* args, FILE* stream);
// print usage as defined by hh_args_t
void
hh_args_print_usage(const hh_args_t* args, FILE* stream, int argc, char* argv[]);

// reads an entire file given by path
// returns a dynamic array with file contents (free with hh_darrfree)
// returns NULL on failure
char* 
hh_read_entire_file(const char* path);
// returns a pointer to the same string that 
// has been advanced past any initial whitespace
const char*
hh_skip_whitespace(const char* str);
// returns truthy when the `str` starts with `prefix`
_Bool
hh_has_prefix(const char* str, const char* prefix);
// returns truthy when the `str` ens with `suffix`
_Bool
hh_has_suffix(const char* str, const char* suffix);
//
#endif // HH__

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

//
//
//

#ifdef HH__
// helper definition for custom log blocks
#ifdef HH_LOG
#define HH_LOG_BLOCK(stream, name) for(uintptr_t \
    HH_LOG_BLOCK_stream  = (uintptr_t) (stream), \
    HH_LOG_BLOCK_toggle  = (uintptr_t) (HH_LOG_APPEND("%s [%s:%d]: ", (name), __FILE__, __LINE__) == 0); \
    HH_LOG_BLOCK_toggle != (uintptr_t) '\n' && HH_LOG_BLOCK_toggle != (uintptr_t) EOF; \
    HH_LOG_BLOCK_toggle  = (uintptr_t) fputc('\n', (FILE*) HH_LOG_BLOCK_stream))
#else
#define HH_LOG_BLOCK(stream, name) if(0)
#endif // HH_LOG

// remove definition of this helper macro
#define HH_STRINGIFY_HELPER(x) #x

// initial capacity of dynamic array
#ifndef HH_ARR_CAP_DEFAULT
#define HH_ARR_CAP_DEFAULT 16
#endif // not HH_ARR_CAP_DEFAULT

// internal array components
typedef struct { 
    size_t len, cap, elem_size; 
} hh_darrheader_t;

// helper macros for dynamic array implementation
#define hh_darrheader(arr)     (((hh_darrheader_t*) arr) - 1)
#define hh_darrnew(arr)        ((arr) = HH__darrnew(HH_ARR_CAP_DEFAULT, sizeof(*arr)))
#define hh_darrgrow(arr, n)    (HH__darrgrow((void**) &(arr), (n), sizeof(*(arr))), (arr))

// helper functions for dynamic array
void*
HH__darrnew(size_t cap, size_t elem_size);
void 
HH__darrgrow(void** arrp, size_t n, size_t elem_size);
size_t
HH__darradd(void** arrp, size_t n, size_t elem_size);
_Bool 
HH__darrswap(void* arrp, size_t i, size_t j);

// arena type
// placed here because the user should never have to interact with it
struct HH__arena {
    char* ptr;
    char* end;
    char* cur;
    hh_arena* next; 
};

// the default size of a 'page' in the allocator
// can be overwritten by the user
#ifndef HH_ARENA_DEFAULT_SIZE
#define HH_ARENA_DEFAULT_SIZE (256 * 1024)
#endif // HH_ARENA_DEFAULT_SIZE

// helper functions for hh_path
char*
HH__path_join(char* path, ...);

// calculate edition using preprocessor
#ifdef __STDC__
#define HH_EDITION 0L
#ifdef __STDC_VERSION__
#ifdef HH_EDITION
#undef HH_EDITION
#endif // HH_EDITION
#define HH_EDITION 1L
#if(__STDC_VERSION__ >= 199409L)
#ifdef HH_EDITION
#undef HH_EDITION
#endif // HH_EDITION
#define HH_EDITION 199409L
#endif // 199409L
#if(__STDC_VERSION__ >= 199901L)
#ifdef HH_EDITION
#undef HH_EDITION
#endif // HH_EDITION
#define HH_EDITION 199901L
#endif // 199901L
#if(__STDC_VERSION__ >= 201112L)
#ifdef HH_EDITION
#undef HH_EDITION
#endif // HH_EDITION
#define HH_EDITION 201112L
#endif // 201112L
#if(__STDC_VERSION__ >= 201710L)
#ifdef HH_EDITION
#undef HH_EDITION
#endif // HH_EDITION
#define HH_EDITION 201710L
#endif // 201710L
#if(__STDC_VERSION__ >= 202311L)
#ifdef HH_EDITION
#undef HH_EDITION
#endif // HH_EDITION
#define HH_EDITION 202311L
#endif // 202311L
#endif // __STDC_VERSION__
#endif // __STD__

// size of the span format specifier buffer
// should not have to modify
#ifndef HH_SPAN_BUF_LEN
#define HH_SPAN_BUF_LEN 32
#endif // not HH_SPAN_BUF_LEN

// NetBSD: getline.c,v 1.2 2014/09/16 17:23:50 christos Exp
ptrdiff_t // NO PREFIX STRIPPING
hh_getdelim(char** buf, size_t* bufsiz, int delimiter, FILE* fp);
ptrdiff_t // NO PREFIX STRIPPING
hh_getline(char** buf, size_t* bufsiz, FILE* fp);

hh_map_entry_t
HH__map_it_begin(const hh_map_t* map);
void
HH__map_it_next(const hh_map_t* map, hh_map_entry_t* entry);

// in practice, this value does not need to be modified
#ifndef HH_ARGS_BUCKET_COUNT
#define HH_ARGS_BUCKET_COUNT 10
#endif // HH_ARGS_BUCKET_COUNT

// defines indentation of the usage tree (must be >2)
#ifndef HH_ARGS_USAGE_INDENT
#define HH_ARGS_USAGE_INDENT 4
#endif // HH_ARGS_USAGE_INDENT

struct HH__args_entry {
    hh_flag_opt flag;
    hh_flag_type type;
    _Bool set;
    union {
        _Bool val_bool;
        const char* val_cstr;
        char* val_path;
        double val_dbl;
        long val_long;
        unsigned long val_ulong;
    } unwrap;
};

struct HH__args_error {
    enum {
        HH__ARGS_ERR_NONE = 0,
        HH__ARGS_ERR_CMD_MISSING,
        HH__ARGS_ERR_CMD_INVALID,
        HH__ARGS_ERR_FLAG_MISSING_VALUE,
        HH__ARGS_ERR_FLAG_INVALID,
        HH__ARGS_ERR_FLAG_DUPLICATE,
        HH__ARGS_ERR_FLAG_REQUIRED,
        HH__ARGS_ERR_FLAG_MISMATCH
    } type;
    const struct HH__args_entry* entry;
    const char* extra;
};

struct HH__args_data {
    hh_arena entries;
    const struct HH__args_entry* entry_help;
    hh_map_t flags;
    hh_map_t flags_long;
    const hh_args_t* deepest_parsed;
    struct HH__args_error error;
};

// TODO: Below is the bucket list for hh_args_t, ordered by precedence
// DONE: * [--help, -h] to print context-specific help menus
// DONE: * required arguments
// DONE: ** error handling when missing required args
// * repeated arguments
// DONE: * error when passing flags that belong to different commands
struct HH__args_t {
    const char* name;
    const char* desc;
    uintptr_t* entries;
    hh_args_t* children;
    hh_args_t* parent;
    struct HH__args_data* data;
};

const void*
HH__args_add_flag(hh_args_t* args, hh_flag_type type, hh_flag_opt opt);
//
#endif // HH__

#ifdef HH_IMPLEMENTATION
// implementation-exclusive includes
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>

// platform-dependent includes
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif // _WIN32

void*
hh_malloc_checked(size_t size) {
    void* ptr = malloc(size);
    HH_ASSERT(ptr != NULL, "Failed to allocate %llu bytes.", (unsigned long long) size);
    return ptr;
}

void*
hh_calloc_checked(size_t num, size_t size) {
    void* ptr = calloc(num, size);
    HH_ASSERT(ptr != NULL, "Failed to allocate %llu bytes.", (unsigned long long) size);
    return ptr;
}

void*
HH__darrnew(size_t cap, size_t elem_size) {
    void* arr = (((hh_darrheader_t*) calloc(1, sizeof(hh_darrheader_t) + elem_size * cap)) + 1);
    hh_darrheader(arr)->len = 0;
    hh_darrheader(arr)->cap = cap;
    hh_darrheader(arr)->elem_size = elem_size;
    return arr;
}

void 
HH__darrgrow(void** arrp, size_t n, size_t elem_size) {
    if(*arrp == NULL) {
        hh_darrheader_t* hdr = calloc(1, sizeof(hh_darrheader_t) + elem_size * HH_MAX(n, HH_ARR_CAP_DEFAULT));
        assert(hdr != NULL);
        hdr->len = 0;
        hdr->cap = HH_MAX(n, HH_ARR_CAP_DEFAULT);
        hdr->elem_size = elem_size;
        *arrp = (void*) (hdr + 1);
        return;
    }
    hh_darrheader_t* hdr = hh_darrheader(*arrp);
    if(hdr->len + n >= hdr->cap) {
        while(hdr->len + n >= hdr->cap) hdr->cap *= 2;
        hdr = realloc(hdr, sizeof(hh_darrheader_t) + hdr->cap * hdr->elem_size);
        assert(hdr != NULL);
        *arrp = (void*) (hdr + 1);
    }
}

size_t
HH__darradd(void** arrp, size_t n, size_t elem_size) {
    HH__darrgrow(arrp, n, elem_size);
    size_t len = hh_darrlen(*arrp);
    if(n) {
        memset((char*) (*arrp) + len * elem_size, 0, elem_size * n);
        hh_darrheader(*arrp)->len = len + n;
    }
    return len;
}

_Bool 
HH__darrswap(void* arrp, size_t i, size_t j) {
    if(i == j) return 1;
    if(arrp == NULL) return 0;
    size_t elem_size = hh_darrheader(arrp)->elem_size;
    char tmp[elem_size];
    memcpy(tmp, ((char*) arrp) + i * elem_size, elem_size);
    memcpy(((char*) arrp) + i * elem_size, ((char*) arrp) + j * elem_size, elem_size);
    memcpy(((char*) arrp) + j * elem_size, tmp, elem_size);
    return 1;
}

void*
hh_arena_alloc(hh_arena* arena, size_t sz) {
    // if this is the first allocation
    if(arena->ptr == NULL) {
        size_t sz_alloc = HH_MAX(HH_ARENA_DEFAULT_SIZE, sz);
        arena->ptr = malloc(sz_alloc);
        if(arena->ptr == NULL) return NULL;
        arena->end = arena->ptr + sz_alloc;
        arena->cur = arena->ptr + sz;
        return arena->ptr;
    }
    // if the requested allocation size is too small to fit in the current segment
    if((size_t) (arena->end - arena->cur) < sz) {
        if(arena->next == NULL) arena->next = calloc(1, sizeof(hh_arena));
        if(arena->next == NULL) return NULL;
        return hh_arena_alloc(arena->next, sz);
    }
    // otherwise, fill in the space in this segment
    void* ptr = arena->cur;
    arena->cur += sz;
    return ptr;
}

void
hh_arena_free(hh_arena* arena) {
    if(arena == NULL) return;
    if(arena->next != NULL) {
        hh_arena_free(arena->next);
        free(arena->next);
    }
    free(arena->ptr);
    memset(arena, 0, sizeof(hh_arena));
}

char* 
hh_path_alloc(const char *raw) {
    char* raw_abs = NULL;
    char* path = NULL;
#ifdef _WIN32
    DWORD len_win = GetFullPathNameA(raw, 0, NULL, NULL);
    if(len_win == 0) return NULL;
    raw_abs = malloc(len_win);
    if(raw_abs == NULL) return NULL;
    if(GetFullPathNameA(raw, len_win, raw_abs, NULL) == 0) {
        free(raw_abs);
        return NULL;
    }
    if(raw_abs[0] >= 'a' && raw_abs[0] <= 'z' && raw_abs[1] == ':')
        raw_abs[0] -= ('a' - 'A');
#else
    raw_abs = realpath(raw, NULL);
    if(raw_abs == NULL) return NULL;
#endif
    hh_darrputstr(path, raw_abs);
    free(raw_abs);
    if(path == NULL) return NULL;
    for(size_t i = 0; path[i]; i++) if(path[i] == '\\') path[i] = '/';
    // length of root path is platform-dependent
#ifdef _WIN32
    size_t len_root = 3;
#else
    size_t len_root = 1;
#endif
    size_t len = hh_darrlen(path);
    if(len > (len_root + 1) && path[len - 2] == '/' && path[len - 1] == '\0') {
        path[len - 2] = '\0';
        hh_darrheader(path)->len -= 1;
    }
    return path;
}

_Bool
hh_path_exists(const char* path) {
    if(path == NULL) return 0;
#ifdef _WIN32
    return _access(path, 0) == 0;
#else
    return access(path, 0) == 0;
#endif
}

_Bool
hh_path_is_file(const char* path) {
    if(path == NULL) return 0;
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
#endif
}

_Bool 
hh_path_is_root(const char* path) {
    if(path == NULL) return 0;
#ifdef _WIN32
    if(hh_darrlen(path) != 4) return 0;
    if(path[0] < 'A' || path[0] > 'Z') return 0;
    return path[1] == ':' && path[2] == '/';
#else
    return hh_darrlen(path) == 2 && path[0] == '/';
#endif
}

char* 
HH__path_join(char* path, ...) {
    if(path == NULL) return NULL;
    va_list args;
    va_start(args, path);
    const char* sub;
    while((sub = va_arg(args, const char*)) != NULL) {
        if(sub[0] == '/' || sub[0] == '\\') ++sub;
        (void) hh_darrpop(path);
        if(hh_darrlast(path) != '/') hh_darrputstr(path, "/");
        hh_darrputstr(path, sub);
        if(hh_darrlen(path) > 2 && hh_darrlast(path) == '/') (void) hh_darrpop(path);
    }
    va_end(args);
    hh_darrput(path, '\0');
    return path;
}

const char*
hh_path_name(const char* path) {
    if(path == NULL) return NULL;
    const char *prev = path;
    for(const char *p = path; *p; ++p) if(*p == '/') prev = p + 1;
    if(prev[0] == '\0') return NULL;
    return prev;
}

char*
hh_path_parent(char* path) {
    if(path == NULL) return NULL;
    if(hh_path_is_root(path)) return NULL;
    while(hh_darrlast(path) != '/') (void) hh_darrpop(path);
#ifdef _WIN32
    size_t len_root = 3;
    _Bool root = path[0] >= 'A' && path[0] <= 'Z' && path[1] == ':' && path[2] == '/';
#else
    size_t len_root = 1;
    _Bool root = path[0] == '/';
#endif
    if(hh_darrlen(path) == len_root && root) {
        if(hh_darrlen(path) == len_root + 1) hh_darrfree(path);
        else hh_darrput(path, '\0');
    } else {
        (void) hh_darrpop(path);
        hh_darrput(path, '\0');
    }
    return path;
}

_Bool
hh_edition_supported(hh_edition_t ed) {
    return HH_EDITION >= ed;
}

_Bool
hh_span_init(hh_span_t* span, const char* ptr, const char* delim) {
    span->ptr = ptr;
    span->delim = delim;
    span->len = 0;
    span->skips = 0;
    while(span->ptr && strchr(" \t\r", span->ptr[0])) span->ptr++;
    return hh_span_next(span);
}

_Bool
hh_span_next(hh_span_t* span) {
    const char* ptr = span->ptr + span->len + span->skips;
    span->len = 0;
    span->skips = 0;
    span->ptr = ptr;
    if(!*ptr) return 0;
    size_t delim_len = span->delim ? strlen(span->delim) : 0;
    if(span->delim) {
        while(*ptr && strncmp(ptr, span->delim, delim_len) != 0 && !strchr(" \t\r\n", *ptr)) ++ptr;
    } else while(*ptr && !strchr(" \t\r\n", *ptr)) ++ptr;
    // ptr now is at the end of the token, pointing to either whitespace or the start of the delimiter
    span->len = (size_t) (ptr - span->ptr);
    if(span->len == 0 && span->delim == NULL) return 0;
    while(*ptr && strchr(" \t\r", *ptr)) ++ptr;
    if(*ptr == '\0') goto finish;
    // either at the delim or a newline
    if(*ptr == '\n') {
        ++ptr;
        goto finish;
    }
    // we are 100% pointing at a delim if there is one
    if(span->delim) {
        if(strncmp(ptr, span->delim, delim_len) != 0) return 0;
        ptr += delim_len;
        while(*ptr && strchr(" \t\r\n", *ptr)) ++ptr;
    }
finish:
    span->skips = (size_t) (ptr - span->ptr) - span->len;
    return 1;
}

_Bool
hh_span_next_line(hh_span_t* span) {
    const char* ptr = span->ptr + span->len + span->skips;
    span->len = 0;
    span->skips = 0;
    while(*ptr && *ptr != '\n') ++ptr;
    if(*ptr == '\n') ++ptr;
    span->ptr = ptr;
    if(*ptr == '\0') return 0;
    return hh_span_next(span);
}

_Bool
hh_span_equals(const hh_span_t span, const char* other) {
    size_t len = strlen(other);
    if(span.len != len) return 0;
    return strncmp(span.ptr, other, span.len) == 0;
}

_Bool
hh_span_parse(const hh_span_t* span, const char* fmt, void* out) {
    HH_ASSERT(fmt[0] == '%', "Unsupported format specifier [%s].", fmt);
    static char fmt_buf[HH_SPAN_BUF_LEN + 1];
    snprintf(fmt_buf, HH_SPAN_BUF_LEN, "%%%llu%s", (unsigned long long) span->len, &fmt[1]);
    return sscanf(span->ptr, fmt_buf, out);
}

_Bool
hh_span_parse_next(hh_span_t* span, const char* fmt, void* out) {
    return hh_span_parse(span, fmt, out) && hh_span_next(span);
}

// adapted from the following link
// https://gist.github.com/MohamedTaha98/ccdf734f13299efb73ff0b12f7ce429f
// thanks to MohamedTaha98
size_t
HH__map_hash_djb2(const void* key, size_t size_key) {
    size_t hash = 5381;
    for (size_t i = 0; i < size_key; ++i) hash = ((hash << 5) + hash) + (size_t) ((char*) key)[i];
    return hash;
}

size_t
HH__map_hash_generic(const hh_map_t* map, const void* key, size_t size_key) {
    return ((map->hash == NULL) ? 
        HH__map_hash_djb2(key, size_key) : 
        (map->hash)(key, size_key)) % map->bucket_count;
}

int
HH__map_comp_generic(const hh_map_t* map, const void* key_query, size_t size_key_query, const void* key_in, size_t size_key_in) {
    if(map->comp != NULL) return (map->comp)(key_query, size_key_query, key_in, size_key_in);
    int result = memcmp(key_query, key_in, HH_MIN(size_key_query, size_key_in));
    if(result != 0) return result;
    if(size_key_query < size_key_in) return -1;
    if(size_key_query > size_key_in) return 1;
    return 0;
}

_Bool 
HH__map_replace(hh_map_t* map, const void* key, size_t size_key, const void* val, size_t size_val) {
    // get the entry, we can only replace if it exists
    hh_map_entry_t entry = hh_map_get(map, key, size_key);
    if(entry.val == NULL) return 0;
    // entry bounds
    char* entry_begin = (char*) entry.key - sizeof(size_t) * 2;
    char* entry_val = (char*) entry.val;
    char* entry_end = entry_val + entry.size_val;
    size_t idx, len;
    idx = HH__map_hash_generic(map, entry.key, entry.size_key);
    // grow array if new entry size is larger
    if(size_val > entry.size_val) {
        char* bucket = map->buckets[idx];
        hh_darradd(map->buckets[idx], size_val - entry.size_val);
        entry_begin = map->buckets[idx] + (entry_begin - bucket);
        entry_val = map->buckets[idx] + (entry_val - bucket);
        entry_end = entry_val + entry.size_val;
    }
    // compute length of tail bytes that we need to slide right
    len = (size_t) ((map->buckets[idx] + hh_darrlen(map->buckets[idx])) - entry_end);
    memmove(entry_val + size_val, entry_end, len);
    // update metadata and replace value
    ((size_t*) entry_begin)[1] = size_val;
    if(val == NULL) memset(entry_val, 0, size_val);
    else memcpy(entry_val, val, size_val);
    return 1;
}

_Bool
hh_map_insert(hh_map_t* map, const void* key, size_t size_key, const void* val, size_t size_val) {
    if(map == NULL) return 0;
    // initialize map
    if(map->buckets == NULL) {
        map->buckets = calloc(map->bucket_count, sizeof(char*));
        if(map->buckets == NULL) return 0;
        for(size_t i = 0; i < map->bucket_count; ++i) hh_darradd(map->buckets[i], sizeof(size_t) * 2);
    } else if(HH__map_replace(map, key, size_key, val, size_val)) return 1;
    // perform insertion
    size_t idx, len;
    idx = HH__map_hash_generic(map, key, size_key);
    len = hh_darrlen(map->buckets[idx]);
    hh_darradd(map->buckets[idx], size_key + size_val + sizeof(size_t) * 2);
    // update entry sizes
    size_t* meta = (((size_t*) (map->buckets[idx] + len)) - 2);
    *(meta++) = size_key;
    *(meta++) = size_val;
    // copy over entry
    memcpy(meta, key, size_key);
    char* val_start = ((char*) meta) + size_key;
    if(val == NULL) memset(val_start, 0, size_val);
    else memcpy(val_start, val, size_val);
    return 1;
}

_Bool
hh_map_insert_entry(hh_map_t* map, const hh_map_entry_t* entry) {
    return hh_map_insert(map, entry->key, entry->size_key, entry->val, entry->size_val);
}

hh_map_entry_t
hh_map_get(const hh_map_t* map, const void* key, size_t size_key) {
    if(map == NULL) return (hh_map_entry_t) {0};
    if(map->buckets == NULL) return (hh_map_entry_t) {0};
    if(key == NULL) return (hh_map_entry_t) {0};
    // get correct bucket
    size_t idx = HH__map_hash_generic(map, key, size_key);
    // step through the bucket
    hh_map_entry_t entry;
    for(size_t i = 0; i < hh_darrlen(map->buckets[idx]);) {
        entry.size_key = *((size_t*) (map->buckets[idx] + i)); i += sizeof(size_t);
        entry.size_val = *((size_t*) (map->buckets[idx] + i)); i += sizeof(size_t);
        entry.key = map->buckets[idx] + i; i += entry.size_key;
        entry.val = map->buckets[idx] + i; i += entry.size_val;
        // return if key was found
        if(HH__map_comp_generic(map, key, size_key, entry.key, entry.size_key) == 0) 
            return entry;
    }
    return (hh_map_entry_t) {0};
}

const void*
hh_map_get_val(const hh_map_t* map, const void* key, size_t size_key) {
    return hh_map_get(map, key, size_key).val;
}

_Bool
hh_map_remove(hh_map_t* map, const void* key, size_t size_key) {
    // get corresponding entry
    hh_map_entry_t entry = hh_map_get(map, key, size_key);
    if(entry.val == NULL) return 0;
    // recompute bucket
    size_t idx = HH__map_hash_generic(map, key, size_key);
    char* bucket = map->buckets[idx];
    size_t len = hh_darrlen(bucket);
    // entry bounds
    char* entry_begin = (char*) entry.key - sizeof(size_t) * 2;
    char* entry_end = (char*) entry.val + entry.size_val;
    // remaining bytes to slide over the current entry
    size_t tail = (size_t) ((bucket + len) - entry_end);
    memmove(entry_begin, entry_end, tail);
    // update hh_darrheader_t length to reflect changes
    hh_darrheader(bucket)->len -= (entry.size_key + entry.size_val + sizeof(size_t) * 2);
    return 1;
}

void
HH__map_it_helper(hh_map_entry_t* entry, const char* entry_begin) {
    entry->size_key = ((size_t*) entry_begin)[0];
    entry->size_val = ((size_t*) entry_begin)[1];
    entry->key = (const void*) (((size_t*) entry_begin) + 2);
    entry->val = (const char*) entry->key + entry->size_key;
}

hh_map_entry_t
HH__map_it_begin(const hh_map_t* map) {
    hh_map_entry_t entry;
    size_t idx;
    // scan all buckets until a non-empty one is found
    for(idx = 0; idx < map->bucket_count; ++idx) {
        // a non-empty bucket is longer than the terminating 0, 0
        if(hh_darrlen(map->buckets[idx]) > sizeof(size_t) * 2) {
            HH__map_it_helper(&entry, map->buckets[idx]);
            return entry;
        }
    }
    // bucket is empty
    return (hh_map_entry_t) {0};
}

void
HH__map_it_next(const hh_map_t* map, hh_map_entry_t* entry) {
    char* entry_begin;
    size_t idx;
    // compute the bucket index of the entry
    idx = HH__map_hash_generic(map, entry->key, entry->size_key);
    // look for the next entry in the same bucket
    entry_begin = ((char*) entry->val) + entry->size_val;
    if(entry_begin + sizeof(size_t) * 2 <= map->buckets[idx] + hh_darrlen(map->buckets[idx])) {
        size_t size_key = ((size_t*) entry_begin)[0];
        if(size_key != 0) goto found;
    }
    // scan remaining buckets if we didn't find another element in the previous one
    for(++idx; idx < map->bucket_count; ++idx) {
        entry_begin = map->buckets[idx];
        if(hh_darrlen(map->buckets[idx]) > sizeof(size_t) * 2) goto found;
    }
    // end of iteration
    memset(entry, 0, sizeof(hh_map_entry_t));
    return;
    // continued iteration
found:
    HH__map_it_helper(entry, entry_begin);
    return;
}

void
hh_map_free(hh_map_t* map) {
    hh_map_entry_t entry;
    if(map->buckets == NULL) return;
    for(size_t i = 0; i < map->bucket_count; ++i) {
        for(size_t j = 0; j < hh_darrlen(map->buckets[i]);) {
            entry.size_key = *((size_t*) (map->buckets[i] + j)); j += sizeof(size_t);
            entry.size_val = *((size_t*) (map->buckets[i] + j)); j += sizeof(size_t);
            entry.key = map->buckets[i] + j; j += entry.size_key;
            entry.val = map->buckets[i] + j; j += entry.size_val;
            if(map->free_key) (map->free_key)(entry.key, entry.size_key);
            if(map->free_val) (map->free_val)(entry.val, entry.size_val);
        }
        hh_darrfree(map->buckets[i]);
    }
    free(map->buckets);
}

static const char*
hh_flag_value_name(hh_flag_opt opt, const hh_flag_type* type) {
    if(type == NULL) return NULL;
    HH_ASSERT_UNREACHABLE(opt.name == NULL || type != HH_FLAG_BOOL);
    if(opt.name != NULL) return opt.name;
    switch(*type) {
    case HH_FLAG_BOOL:  return NULL;
    case HH_FLAG_CSTR:  return "string";
    case HH_FLAG_PATH:  return "path";
    case HH_FLAG_DBL:   return "number";
    case HH_FLAG_LONG:  return "int";
    case HH_FLAG_ULONG: return "uint";
    default: HH_UNREACHABLE;
    }
    return NULL;
}

#define FLAG_FMT "%s-%s%.*s%s%s%s%s%s%s"
#define FLAG_FMT_ARGS(opt, type) \
        ((opt).required) ? "" : "[", \
        "", \
        ((opt).flag == '\0') ? 0 : 1, \
        ((opt).flag == '\0') ? "" : &(opt).flag, \
        ((opt).flag == '\0') ? "-" : (((opt).flag_long == NULL) ? "" : ", --"), \
        ((opt).flag_long == NULL) ? "" : (opt).flag_long, \
        ((type) == NULL || *((hh_flag_type*) (type)) == HH_FLAG_BOOL) ? "" : " <", \
        ((type) == NULL || *((hh_flag_type*) (type)) == HH_FLAG_BOOL) ? "" : hh_flag_value_name(opt, type), \
        ((type) == NULL || *((hh_flag_type*) (type)) == HH_FLAG_BOOL) ? "" : ">", \
        ((opt).required) ? "" : "]"
#define FLAG_FMT_ARGS_SHORT(opt, type) \
        ((opt).required) ? "" : "[", \
        ((opt).flag == '\0') ? "-" : "", \
        ((opt).flag == '\0') ? ((int) strlen((opt).flag_long)) : 1, \
        ((opt).flag == '\0') ? (opt).flag_long : &(opt).flag, \
        "", \
        "", \
        ((type) == NULL || *((hh_flag_type*) (type)) == HH_FLAG_BOOL) ? "" : " <", \
        ((type) == NULL || *((hh_flag_type*) (type)) == HH_FLAG_BOOL) ? "" : hh_flag_value_name(opt, type), \
        ((type) == NULL || *((hh_flag_type*) (type)) == HH_FLAG_BOOL) ? "" : ">", \
        ((opt).required) ? "" : "]"

#define FLAG_INVALID "Invalid hh_args_t configuration. "

struct HH__args_entry*
hh_args_data_add_flag(struct HH__args_data* data, hh_flag_type type, hh_flag_opt opt) {
    // fail assert early if no flag is provided
    HH_ASSERT(opt.flag != '\0' || opt.flag_long != NULL, 
        FLAG_INVALID "Either short or long flag must be set");
    // ensure flag and flag_long don't already exist
    if(opt.flag != '\0')
        HH_ASSERT(!hh_map_get_val(&data->flags, &opt.flag, 1), 
            FLAG_INVALID "Flag '" FLAG_FMT "' already added", FLAG_FMT_ARGS(opt, NULL));
    if(opt.flag_long != NULL)
        HH_ASSERT(!hh_map_get_val_with_cstr_key(&data->flags_long, opt.flag_long),
            FLAG_INVALID "Long flag '" FLAG_FMT "' already added", FLAG_FMT_ARGS(opt, NULL));
    // provided 'name' to boolean
    if(type == HH_FLAG_BOOL) {
        HH_ASSERT(opt.name == NULL, 
            FLAG_INVALID "Provided value name to boolean flag '" FLAG_FMT "'", FLAG_FMT_ARGS(opt, NULL));
        HH_ASSERT(!opt.required, 
            FLAG_INVALID "Boolean flag '" FLAG_FMT "' can't be required: ", FLAG_FMT_ARGS(opt, NULL));
    }
    // allocate
    struct HH__args_entry* entry = hh_arena_alloc(&data->entries, sizeof(*entry));
    if(entry == NULL) return NULL;
    // insert into hashmaps
    if(opt.flag != '\0')
        HH_ASSERT(hh_map_insert(&data->flags, &opt.flag, 1, &entry, sizeof(uintptr_t)), 
            FLAG_INVALID "Failed to add flag '" FLAG_FMT "' to hh_args_t", FLAG_FMT_ARGS(opt, NULL));
    if(opt.flag_long != NULL)
        HH_ASSERT(hh_map_insert_with_cstr_key(&data->flags_long, opt.flag_long, &entry, sizeof(uintptr_t)),
            FLAG_INVALID "Failed to add long flag '" FLAG_FMT "' to hh_args_t: ", FLAG_FMT_ARGS(opt, NULL));
    entry->flag = opt;
    entry->type = type;
    return entry;
}

static struct HH__args_data*
hh_args_data_init(void) {
    struct HH__args_data* data = calloc(1, sizeof(*data));
    HH_ASSERT(data != NULL, FLAG_INVALID "Failed to allocate");
    data->flags.bucket_count = HH_ARGS_BUCKET_COUNT;
    data->flags_long.bucket_count = HH_ARGS_BUCKET_COUNT;
    // add [-h, --help] flag
    static const hh_flag_opt opt = { 
        .flag = 'h', 
        .flag_long = "help", 
        .desc = "show this help menu" 
    };
    data->entry_help = hh_args_data_add_flag(data, HH_FLAG_BOOL, opt);
    HH_ASSERT(data->entry_help != NULL, "Failed to add flag '" FLAG_FMT "'", FLAG_FMT_ARGS(opt, NULL));
    return data;
}

const void*
HH__args_add_flag(hh_args_t* args, hh_flag_type type, hh_flag_opt opt) {
    HH_ASSERT(args != NULL, "hh_args_t was NULL");
    // initialize
    if(args->data == NULL) args->data = hh_args_data_init();
    // add flag and return handle to the value
    struct HH__args_entry* entry = hh_args_data_add_flag(args->data, type, opt);
    hh_darrput(args->entries, (uintptr_t) entry);
    switch(entry->type) {
    case HH_FLAG_BOOL:  return &entry->unwrap.val_bool;
    case HH_FLAG_CSTR:  return &entry->unwrap.val_cstr;
    case HH_FLAG_PATH:  return &entry->unwrap.val_path;
    case HH_FLAG_DBL:   return &entry->unwrap.val_dbl;
    case HH_FLAG_LONG:  return &entry->unwrap.val_long;
    case HH_FLAG_ULONG: return &entry->unwrap.val_ulong;
    default: HH_UNREACHABLE;
    }
}

hh_args_t*
hh_args_add_cmd(hh_args_t* args, const char* name, const char* desc) {
    HH_ASSERT(name != NULL, FLAG_INVALID "Must provide command name");
    // create a new child node
    size_t idx = hh_darradd(args->children, 1);
    hh_args_t* child = &args->children[idx];
    // populate it
    child->name = name;
    child->desc = desc;
    child->parent = args;
    child->data = args->data;
    return child;
}

#undef FLAG_INVALID

#define ERR_SET(args, ...) \
    args->data->error = ((struct HH__args_error) { __VA_ARGS__ });

static hh_args_t*
hh_args_root(hh_args_t* args) {
    hh_args_t* root = args;
    while(root->parent != NULL) root = root->parent;
    return root;
}

static struct HH__args_entry*
hh_args_parse_entry_inclusive(hh_args_t* args, const char* argi, const char** val) {
    HH_ASSERT_UNREACHABLE(args != NULL);
    if(argi == NULL) return NULL;
    size_t len = strlen(argi);
    const char* argi_split = NULL;
    const void* ptr;
    if(len > 2 && hh_has_prefix(argi, "--")) {
        // check if there is an equals
        if((argi_split = strchr(argi, '=')))
            ptr = hh_map_get_val(&args->data->flags_long, argi + 2, (size_t) (argi_split - argi - 2));
        else ptr = hh_map_get_val_with_cstr_key(&args->data->flags_long, argi + 2);
    } else if(len > 1 && argi[0] == '-') {
        if(len > 2) argi_split = argi + 1;
        ptr = hh_map_get_val(&args->data->flags, argi + 1, 1);
    } else return 0;
    if(ptr == NULL) return 0;
    // check if the value is part of this argument
    if(argi_split != NULL && val != NULL) val[0] = argi_split + 1;
    return ((struct HH__args_entry**) ptr)[0];
}

static struct HH__args_entry*
hh_args_parse_entry(hh_args_t* args, const char* argi, const char** val) {
    struct HH__args_entry* query = hh_args_parse_entry_inclusive(args, argi, val);
    if(query != NULL) {
        const struct HH__args_entry* entry;
        for(size_t i = 0; i < hh_darrlen(args->entries); ++i) {
            entry = (const struct HH__args_entry*) args->entries[i];
            if(query == entry) return query;
        }
    }
    return NULL;
}

static _Bool
hh_args_parse_inner(hh_args_t* args, int argc, char* argv[]) {
    // verify that we received enough commands/subcommands
    if(argc == 0 && hh_darrlen(args->children) > 0) {
        ERR_SET(args, .type = HH__ARGS_ERR_CMD_MISSING);
        return 0;
    }
    // invoke subcommands
    _Bool found = !hh_darrlen(args->children);
    for(size_t i = 0; i < hh_darrlen(args->children); ++i) {
        if(strcmp(argv[0], args->children[i].name) != 0) continue;
        // set the child node as the latest parsed
        args->data->deepest_parsed = &args->children[i];
        if(!hh_args_parse_inner(&args->children[i], argc - 1, argv + 1)) return 0;
        found = 1;
    }
    // error if we failed to find one
    struct HH__args_entry* entry;
    if(!found) {
        entry = hh_args_parse_entry(hh_args_root(args), argv[0], NULL);
        ERR_SET(args, 
            .type = HH__ARGS_ERR_CMD_INVALID,
            .entry = entry,
            .extra = argv[0]);
        return 0;
    }
    // attempt to parse all arguments from argv[1] onwards
    const char* ptr;
    char* end;
    for(ptr = NULL; argv[0]; ++argv, ptr = NULL, entry = NULL) {
        entry = hh_args_parse_entry(args, argv[0], &ptr);
        if(entry == NULL) continue;
        if(entry->set) {
            ERR_SET(args,
                .type = HH__ARGS_ERR_FLAG_DUPLICATE,
                .entry = entry);
            return 0;
        }
        if(entry->type == HH_FLAG_BOOL) {
            entry->unwrap.val_bool = 1;
            entry->set = 1;
            goto done;
        }
        // grab the next arg
        if(ptr == NULL) {
            if(argv[1] == NULL) {
                ERR_SET(args, 
                    .type = HH__ARGS_ERR_FLAG_MISSING_VALUE,
                    .entry = entry);
                return 0;
            }
            ptr = (++argv)[0];
        }
        switch(entry->type) {
        case HH_FLAG_BOOL: HH_UNREACHABLE;
        case HH_FLAG_CSTR:
            entry->unwrap.val_cstr = ptr;
            break;
        case HH_FLAG_PATH:
            entry->unwrap.val_path = hh_path_alloc(ptr);
            // invalid path provided
            if(entry->unwrap.val_path == NULL) goto invalid;
            break;
        case HH_FLAG_DBL:
            errno = 0;
            entry->unwrap.val_dbl = strtod(ptr, &end);
            // invalid float provided
            if(ptr == end || errno == ERANGE) goto invalid;
            break;
        case HH_FLAG_LONG:
            errno = 0;
            entry->unwrap.val_long = strtol(ptr, &end, 0);
            // invalid long provided
            if(ptr == end || errno == ERANGE) goto invalid;
            break;
        case HH_FLAG_ULONG: {
            errno = 0;
            long temp = strtol(ptr, &end, 0);
            // invalid unsigned long provided
            if(temp < 0 || ptr == end || errno == ERANGE) goto invalid;
            entry->unwrap.val_ulong = (unsigned long) temp;
        } break;
        default: HH_UNREACHABLE;
        }
        entry->set = 1;
    }
    // ensure no required flags were missed at this level
done:
    for(size_t i = 0; i < hh_darrlen(args->entries); ++i) {
        entry = (struct HH__args_entry*) args->entries[i];
        if(entry->flag.required && !entry->set) {
            ERR_SET(args,
                .type = HH__ARGS_ERR_FLAG_REQUIRED,
                .entry = entry);
            return 0;
        }
    }
    return 1;
    // set error for an incorrectly parsed flag value
invalid:
    ERR_SET(args, 
        .type = HH__ARGS_ERR_FLAG_INVALID,
        .entry = entry,
        .extra = ptr);
    return 0;
}

_Bool
hh_args_parse(hh_args_t* args, FILE* stream, int argc, char* argv[]) {
    HH_ASSERT(args != NULL && argc > 0 && argv[argc] == NULL, "Parameters were malformed");
    HH_ASSERT(args->parent == NULL, "Passed non-root hh_args_t to hh_args_parse");
    args->data->deepest_parsed = args;
    _Bool result = hh_args_parse_inner(args, argc - 1, argv + 1);
    // disregard error if [-h, --help] flag was passed
    for(int i = 0; i < argc; ++i) {
        if(args->data->entry_help != hh_args_parse_entry_inclusive(args, argv[i], NULL)) continue;
        hh_args_print_usage(args, stream, argc, argv);
        return 1;
    }
    if(!result) return 0;
    // ensure no flags corresponding to unused commands are passed
    const struct HH__args_entry* entry;
    for(int i = 0; i < argc; ++i) {
        entry = hh_args_parse_entry_inclusive(args, argv[i], NULL);
        if(entry == NULL) continue;
        if(!entry->set) {
            ERR_SET(args,
                .type = HH__ARGS_ERR_FLAG_MISMATCH,
                .entry = entry);
            return 0;
        }
    }
    return 1;
}

#undef ERR_SET

void
hh_args_free(hh_args_t* args) {
    // free the underlying entries
    struct HH__args_entry* entry;
    for(size_t i = 0; i < hh_darrlen(args->entries); ++i) {
        entry = (struct HH__args_entry*) args->entries[i];
        switch(entry->type) {
        case HH_FLAG_BOOL:
        case HH_FLAG_CSTR:  break;
        case HH_FLAG_PATH:  hh_path_free(entry->unwrap.val_path); break;
        case HH_FLAG_DBL:
        case HH_FLAG_LONG:
        case HH_FLAG_ULONG: break;
        default: HH_UNREACHABLE;
        }
    }
    hh_darrfree(args->entries);
    // then recursively call this nodes children
    for(size_t i = 0; i < hh_darrlen(args->children); ++i)
        hh_args_free(&args->children[i]);
    hh_darrfree(args->children);
    // lastly, if root, free the data backing the argument tree
    if(args->parent == NULL) {
        hh_arena_free(&args->data->entries);
        hh_map_free(&args->data->flags);
        hh_map_free(&args->data->flags_long);
    }
}

#if 0
void
hh_args_print_error_raw(const hh_args_t* args, FILE* stream) {
    // data->error.type
    fprintf(stream, "data->error.type: ");
    switch(args->data->error.type) {
    case HH__ARGS_ERR_NONE:
        fprintf(stream, "HH__ARGS_ERR_NONE\n"); 
        break;
    case HH__ARGS_ERR_CMD_MISSING: 
        fprintf(stream, "HH__ARGS_ERR_CMD_MISSING\n"); 
        break;
    case HH__ARGS_ERR_CMD_INVALID: 
        fprintf(stream, "HH__ARGS_ERR_CMD_INVALID\n"); 
        break;
    case HH__ARGS_ERR_FLAG_MISSING_VALUE: 
        fprintf(stream, "HH__ARGS_ERR_FLAG_MISSING_VALUE\n"); 
        break;
    case HH__ARGS_ERR_FLAG_INVALID: 
        fprintf(stream, "HH__ARGS_ERR_FLAG_INVALID\n"); 
        break;
    case HH__ARGS_ERR_FLAG_DUPLICATE: 
        fprintf(stream, "HH__ARGS_ERR_FLAG_DUPLICATE\n"); 
        break;
    case HH__ARGS_ERR_FLAG_REQUIRED: 
        fprintf(stream, "HH__ARGS_ERR_FLAG_REQUIRED\n"); 
        break;
    case HH__ARGS_ERR_FLAG_MISMATCH: 
        fprintf(stream, "HH__ARGS_ERR_FLAG_MISMATCH\n"); 
        break;
    default: HH_UNREACHABLE;
    }
    fprintf(stream, "\n");
    // data->error.entry
    fprintf(stream, "data->error.entry: ");
    if(args->data->error.entry != NULL) fprintf(stream, FLAG_FMT, FLAG_FMT_ARGS(args->data->error.entry->flag));
    else fprintf(stream, "(null)");
    fprintf(stream, "\n");
    // data->error.extra
    fprintf(stream, "data->error.extra: %s\n", args->data->error.extra);
    // data->deepest_parsed
    fprintf(stream, "data->deepest_parsed: ");
    if(args->data->deepest_parsed != NULL && args->data->deepest_parsed->name != NULL) fprintf(stream, "%s", args->data->deepest_parsed->name);
    else fprintf(stream, "(null)");
    fprintf(stream, "\n");
}
#endif

static void
hh_args_print_error_helper(const hh_args_t* origin, FILE* stream) {
    size_t len = hh_darrlen(origin->children);
    if(origin->parent == NULL) {
        fprintf(stream, "command");
    } else {
        HH_ASSERT_UNREACHABLE(origin->name != NULL);
        fprintf(stream, "subcommand for '%s'", origin->name);
    }
    fputc(' ', stream);
    HH_ASSERT_UNREACHABLE(len > 0);
    fprintf(stream, "[must be one of: ");
    for(size_t i = 0; i < len; ++i) {
        fprintf(stream, "%s%s", origin->children[i].name, (i + 1 < len) ? ", " : "");
    }
    fprintf(stream, "]");
}

void
hh_args_print_error(const hh_args_t* args, FILE* stream) {
    HH_ASSERT(args != NULL && stream != NULL, "Parameters were malformed");
    const struct HH__args_error* err = &args->data->error;
    const hh_args_t* origin = args->data->deepest_parsed;
    HH_ASSERT_UNREACHABLE(origin != NULL);
    // print descriptive error message
    switch(err->type) {
    case HH__ARGS_ERR_NONE: return;
    case HH__ARGS_ERR_CMD_MISSING:
        fprintf(stream, "Missing required ");
        hh_args_print_error_helper(origin, stream);
        break;
    case HH__ARGS_ERR_CMD_INVALID:
        if(err->entry != NULL) 
            fprintf(stream, "Provided argument before required");
        else fprintf(stream, "Invalid");
        fputc(' ', stream);
        hh_args_print_error_helper(origin, stream);
        break;
    case HH__ARGS_ERR_FLAG_MISSING_VALUE:
        HH_ASSERT_UNREACHABLE(err->entry != NULL);
        fprintf(stream, "Flag '" FLAG_FMT "' is missing a required value", 
            FLAG_FMT_ARGS(err->entry->flag, &err->entry->type));
        break;
    case HH__ARGS_ERR_FLAG_INVALID:
        HH_ASSERT_UNREACHABLE(err->entry != NULL);
        fprintf(stream, "Flag '" FLAG_FMT "' received an invalid value: %s", 
            FLAG_FMT_ARGS(err->entry->flag, &err->entry->type), err->extra);
        break;
    case HH__ARGS_ERR_FLAG_DUPLICATE:
        HH_ASSERT_UNREACHABLE(err->entry != NULL);
        fprintf(stream, "Flag '" FLAG_FMT "' is passed more than once", 
            FLAG_FMT_ARGS(err->entry->flag, &err->entry->type));
        break;
    case HH__ARGS_ERR_FLAG_REQUIRED:
        HH_ASSERT_UNREACHABLE(err->entry != NULL);
        fprintf(stream, "Required flag '" FLAG_FMT "' is missing", 
            FLAG_FMT_ARGS(err->entry->flag, &err->entry->type));
        break;
    case HH__ARGS_ERR_FLAG_MISMATCH:
        HH_ASSERT_UNREACHABLE(origin->parent != NULL);
        HH_ASSERT_UNREACHABLE(origin->name != NULL);
        fprintf(stream, "Flag '" FLAG_FMT "' not supported by provided '%s' %s", 
            FLAG_FMT_ARGS(err->entry->flag, &err->entry->type),
            origin->name, origin->parent->parent ? "subcommand" : "command");
        break;
    default: HH_UNREACHABLE;
    }
}

#undef FLAG_FMT_ARGS

static size_t
hh_flag_width(const struct HH__args_entry* entry) {
    size_t col = 0;
    if(!entry->flag.required) col += 2;
    col += 2;
    if(entry->flag.flag == '\0') {
        HH_ASSERT_UNREACHABLE(entry->flag.flag_long != NULL);
        col += strlen(entry->flag.flag_long);
    }
    const char* name = hh_flag_value_name(entry->flag, &entry->type);
    if(name != NULL) col += 3 + strlen(name);
    return col;
}

#define USAGE_OUT(...) if(padding > 0) fprintf(stream, __VA_ARGS__)

static size_t
hh_args_print_usage_entry(const struct HH__args_entry* entry, FILE* stream, 
    _Bool* levels, size_t padding) {
    size_t col = 0;
    for(size_t i = 0; i < hh_darrlen(levels); ++i) {
        USAGE_OUT("%s%*s", levels[i] ? "│" : " ", HH_ARGS_USAGE_INDENT - 2, "");
        col += HH_ARGS_USAGE_INDENT - 1;
    }
    USAGE_OUT(FLAG_FMT, FLAG_FMT_ARGS_SHORT(entry->flag, &entry->type));
    col += hh_flag_width(entry);
    HH_ASSERT_UNREACHABLE(padding == 0 || col <= padding);
    USAGE_OUT("%*s", (int) (padding - col - 1), "");
    if(entry->flag.desc != NULL) {
        USAGE_OUT("%s", entry->flag.desc);
        if(entry->flag.flag != '\0' && entry->flag.flag_long != NULL) {
            USAGE_OUT(" (alt: ");
            if(!entry->flag.required) USAGE_OUT("["); 
            USAGE_OUT("--%s", entry->flag.flag_long);
            if(!entry->flag.required) USAGE_OUT("]"); 
            USAGE_OUT(")"); 
        }
    }
    USAGE_OUT("\n");
    return col;
}

static size_t
hh_args_print_usage_inner(const hh_args_t* args, FILE* stream,
    int argc, char* argv[], _Bool** levels, int last, size_t padding) {
    HH_ASSERT_UNREACHABLE(HH_ARGS_USAGE_INDENT > 2);
    size_t col = 0;
    for(size_t i = 0; i + 1 < hh_darrlen(*levels); ++i) {
        USAGE_OUT("%s%*s", (*levels)[i] ? "│" : " ", HH_ARGS_USAGE_INDENT - 2, "");
        col += HH_ARGS_USAGE_INDENT - 1;
    }
    if(args->parent != NULL) {
        USAGE_OUT("%s", last ? "└" : "├");
        for(size_t i = 0; i < HH_ARGS_USAGE_INDENT - 2; ++i) USAGE_OUT("─");
        USAGE_OUT("%s", args->name);
        col += HH_ARGS_USAGE_INDENT + strlen(args->name);
    } else {
        const char* exe = hh_path_name(argv[0]);
        USAGE_OUT("./%s", exe);
        col += 2 + strlen(exe);
    }
    // print description aligned to the second column
    HH_ASSERT_UNREACHABLE(padding == 0 || col <= padding);
    if(args->desc != NULL) {
        USAGE_OUT("%*s%s", (int) (padding - col), "", args->desc);
    } else if(args->parent == NULL) {
        USAGE_OUT("%*sDESCRIPTION", (int) (padding - col - 1), "");
    }
    USAGE_OUT("\n");
    // print flags
    hh_darrput(*levels, hh_darrlen(args->children) != 0);
    const struct HH__args_entry* entry;
    for(size_t i = 0, j; i < hh_darrlen(args->entries); ++i) {
        entry = (const struct HH__args_entry*) args->entries[i];
        j = hh_args_print_usage_entry(entry, stream, *levels, padding);
        col = HH_MAX(col, j);
    }
    (void) hh_darrpop(*levels);
    // print subcommands
    for(size_t i = 0, j = hh_darrlen(args->children), k; i < j; ++i) {
        hh_darrput(*levels, i != j - 1);
        k = hh_args_print_usage_inner(&args->children[i], stream,
            argc, argv, levels, i == (j - 1), padding);
        col = HH_MAX(col, k);
        (void) hh_darrpop(*levels);
    }
    return col;
}

#undef USAGE_OUT

static void
hh_args_print_synopsis_cmds(const hh_args_t *args, FILE *stream, int argc, char *argv[]) {
    HH_ASSERT_UNREACHABLE(args != NULL);
    if(args->parent != NULL) {
        hh_args_print_synopsis_cmds(args->parent, stream, argc, argv);
        fprintf(stream, "%s ", args->name);
    } else {
        fprintf(stream, "./%s ", hh_path_name(argv[0]));
    }
}

static void
hh_args_print_synopsis_flags(const hh_args_t *args, FILE *stream) {
    HH_ASSERT_UNREACHABLE(args != NULL);
    if(args->parent != NULL) {
        hh_args_print_synopsis_flags(args->parent, stream);
    }
    const struct HH__args_entry* entry;
    for(size_t i = 0; i < hh_darrlen(args->entries); ++i) {
        entry = (const struct HH__args_entry*) args->entries[i];
        fprintf(stream, FLAG_FMT " ", FLAG_FMT_ARGS_SHORT(entry->flag, &entry->type));
    }
}

#undef FLAG_FMT
#undef FLAG_FMT_ARGS_SHORT

static void
hh_args_print_synopsis(const hh_args_t *args, FILE *stream, int argc, char *argv[]) {
    while(hh_darrlen(args->children) == 1) args = &args->children[0];
    hh_args_print_synopsis_cmds(args, stream, argc, argv);
    HH_ASSERT_UNREACHABLE(hh_darrlen(args->children) != 1);
    _Bool cont = 0;
    if(hh_darrlen(args->children) > 1) {
        _Bool sub_one = 0;
        _Bool sub_all = 1;
        _Bool temp;
        if(args->parent == NULL) {
            fprintf(stream, "<command> ");
            for(size_t i = 0; i < hh_darrlen(args->children); ++i) {
                temp = (hh_darrlen(args->children[i].children) != 0);
                sub_one &= temp;
                sub_all |= temp;
            }
            if(sub_one) {
                fprintf(stream, "%s<subcommands>...%s ",
                    sub_all ? "[" : "",
                    sub_all ? "]" : "");
            } else if(sub_all) fprintf(stream, "[<subcommands>[...]] ");
        } else {
            fprintf(stream, "<subcommand>");
            for(size_t i = 0; i < hh_darrlen(args->children); ++i) {
                temp = (hh_darrlen(args->children[i].children) != 0);
                sub_all &= temp;
                sub_one |= temp;
            }
            if(sub_all) fprintf(stream, "...");
            else if(sub_one) fprintf(stream, "[...]");
            fputc(' ', stream);
        }
        cont = 1;
    }
    hh_args_print_synopsis_flags(args, stream);
    if(cont) fprintf(stream, "...");
    fputc('\n', stream);
}

void
hh_args_print_usage(const hh_args_t* args, FILE* stream, int argc, char* argv[]) {
    HH_ASSERT(args != NULL && argc > 0 && argv[argc] == NULL, "Parameters were malformed");
    HH_ASSERT(args->parent == NULL, "Passed non-root hh_args_t to hh_args_parse");
    fprintf(stream, "SYNOPSIS\n");
    hh_args_print_synopsis(args->data->deepest_parsed, stream, argc, argv);
    fputc('\n', stream);
    _Bool* levels = NULL;
    size_t padding = hh_args_print_usage_inner(args, stream, argc, argv, 
        &levels, 1, 0);
    hh_darrclear(levels);
    hh_args_print_usage_inner(args, stream, argc, argv, 
        &levels, 1, padding + HH_ARGS_USAGE_INDENT * 2);
    hh_darrfree(levels);
}

char* 
hh_read_entire_file(const char* path) {
    FILE* f = fopen(path, "rb");
    char* f_buf = NULL;
    if(f == NULL) {
        HH_ERR("Failed to open file at path [%s].", path);
        return NULL;
    }
    if(fseek(f, 0, SEEK_END)) {
        HH_ERR("Failed to seek to end of file while reading [%s].", path);
        goto failure;
    }
    long size_temp = ftell(f);
    if(size_temp < 0) {
        HH_ERR("Failed to read file size [%s].", path);
        goto failure;
    }
    unsigned long size = (unsigned long) size_temp;
    rewind(f);
    (void) hh_darradd(f_buf, size);
    if(f_buf == NULL) {
        HH_ERR("Failed to allocate buffer for file contents [%s].", path);
        goto failure;
    }
    size_t read_size = fread(f_buf, 1, size, f);
    if(read_size != (size_t) size) {
        HH_ERR("Failed to read entire file into buffer [%s].", path);
        goto failure;
    }
    (void) hh_darradd(f_buf, '\0');
    fclose(f);
    return f_buf;
failure:
    fclose(f);
    hh_darrfree(f_buf);
    return NULL;
}

const char*
hh_skip_whitespace(const char* ptr) {
    while(strchr(" \t\r\n", *ptr) && (*ptr) != '\0') ++ptr;
    return ptr;
}

_Bool
hh_has_prefix(const char* str, const char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

_Bool
hh_has_suffix(const char* str, const char* suffix) {
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    return len_suffix <= len_str && !strcmp(str + len_str - len_suffix, suffix);
}

/*-
 * Copyright (c) 2011 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Christos Zoulas.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

ptrdiff_t
hh_getdelim(char** buf, size_t* bufsiz, int delimiter, FILE* fp) {
    char *ptr, *eptr;
    if(*buf == NULL || *bufsiz == 0) {
        *bufsiz = BUFSIZ;
        if((*buf = (char*) malloc(*bufsiz)) == NULL) return -1;
    }
    for(ptr = *buf, eptr = *buf + *bufsiz;;) {
        int c = fgetc(fp);
        if(c == -1) {
            if(feof(fp)) {
                ptrdiff_t diff = ptr - *buf;
                if(diff != 0) {
                    *ptr = '\0';
                    return diff;
                }
            }
            return -1;
        }
        *ptr++ = (char) c;
        if(c == delimiter) {
            *ptr = '\0';
            return ptr - *buf;
        }
        if(ptr + 2 >= eptr) {
            char *nbuf;
            size_t nbufsiz = *bufsiz * 2;
            ptrdiff_t d = ptr - *buf;
            if((nbuf = (char*) realloc(*buf, nbufsiz)) == NULL) return -1;
            *buf = nbuf;
            *bufsiz = nbufsiz;
            eptr = nbuf + nbufsiz;
            ptr = nbuf + d;
        }
    }
}

ptrdiff_t
hh_getline(char** buf, size_t* bufsiz, FILE* fp) {
    return hh_getdelim(buf, bufsiz, '\n', fp);
}
//
#endif // HH_IMPLEMENTATION

#ifndef HH__STRIP_PREFIXES
//
#define HH__STRIP_PREFIXES
//
#ifdef HH_STRIP_PREFIXES
#define MAX HH_MAX
#define MIN HH_MIN
#define ARR_LEN HH_ARR_LEN
#define UNUSED HH_UNUSED
#define FALLTHROUGH HH_FALLTHROUGH
#define DBG HH_DBG
#define MSG HH_MSG
#define ERR HH_ERR
#define DBG_BLOCK HH_DBG_BLOCK
#define MSG_BLOCK HH_MSG_BLOCK
#define ERR_BLOCK HH_ERR_BLOCK
#define LOG_APPEND HH_LOG_APPEND
#define STRINGIFY HH_STRINGIFY
#define STRINGIFY_BOOL HH_STRINGIFY_BOOL
#define ASSERT_BEFORE HH_ASSERT_BEFORE
#define ASSERT HH_ASSERT
#define ASSERT_UNREACHABLE HH_ASSERT_UNREACHABLE
#define UNREACHABLE HH_UNREACHABLE
#define malloc_checked hh_malloc_checked
#define calloc_checked hh_calloc_checked
#define fp_wrap_t hh_fp_wrap_t
#define fp_wrap hh_fp_wrap
#define fp_unwrap hh_fp_unwrap
#define darrclear hh_darrclear
#define darrfree hh_darrfree
#define darrlast hh_darrlast
#define darrput hh_darrput
#define darrpop hh_darrpop
#define darradd hh_darradd
#define darrlen hh_darrlen
#define darrsetlen hh_darrsetlen
#define darrcap hh_darrcap
#define darrswap hh_darrswap
#define darrswapdel hh_darrswapdel
#define darrputstr hh_darrputstr
#define arena hh_arena
#define arena_alloc hh_arena_alloc
#define arena_free hh_arena_free
#define path_alloc hh_path_alloc
#define path_exists hh_path_exists
#define path_is_file hh_path_is_file
#define path_is_root hh_path_is_root
#define path_join hh_path_join
#define path_name hh_path_name
#define path_parent hh_path_parent
#define path_free hh_path_free
#define EDITION_89 HH_EDITION_89
#define EDITION_90 HH_EDITION_90
#define EDITION_94 HH_EDITION_94
#define EDITION_99 HH_EDITION_99
#define EDITION_11 HH_EDITION_11
#define EDITION_17 HH_EDITION_17
#define EDITION_23 HH_EDITION_23
#define edition_t hh_edition_t
#define edition_supported hh_edition_supported
#define EDITION_SUPPORTED HH_EDITION_SUPPORTED
#define span_t hh_span_t
#define span_init hh_span_init
#define span_next hh_span_next
#define span_next_line hh_span_next_line
#define span_parse hh_span_parse
#define span_parse_next hh_span_parse_next
#define span_equals hh_span_equals
#define map_hash_f hh_map_hash_f
#define map_comp_f hh_map_comp_f
#define map_free_f hh_map_free_f
#define map_t hh_map_t
#define map_entry_t hh_map_entry_t
#define map_insert hh_map_insert
#define map_insert_with_cstr_key hh_map_insert_with_cstr_key
#define map_insert_entry hh_map_insert_entry
#define map_get hh_map_get
#define map_get_with_cstr_key hh_map_get_with_cstr_key
#define map_get_val hh_map_get_val
#define map_remove hh_map_remove
#define map_it hh_map_it
#define map_free hh_map_free
#define args_t hh_args_t
#define flag_type hh_flag_type
#define flag_opt hh_flag_opt
#define args_add_flag hh_args_add_flag
#define args_add_cmd hh_args_add_cmd
#define args_parse hh_args_parse
#define args_free hh_args_free
#define args_print_error hh_args_print_error
#define args_print_usage hh_args_print_usage
#define read_entire_file hh_read_entire_file
#define skip_whitespace hh_skip_whitespace
#define has_prefix hh_has_prefix
#define has_suffix hh_has_suffix
//
#endif // HH_STRIP_PREFIXES
//
#endif // HH__STRIP_PREFIXES
