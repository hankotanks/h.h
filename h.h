#ifndef HH__
#define HH__

#ifndef _WIN32
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif // not _DEFAULT_SOURCE
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif // _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#else
#ifdef __MINGW32__
#ifdef _MSC_VER
#ifndef __USE_MINGW_ANSI_STDIO
#define __USE_MINGW_ANSI_STDIO
#endif // not __USE_MINGW_ANSI_STDIO
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
#endif // not HH_ERR_STREAM
#ifndef HH_MSG_STREAM
#define HH_MSG_STREAM stdout
#endif // not HH_MSG_STREAM
#ifndef HH_DBG_STREAM
#define HH_DBG_STREAM stdout
#endif // not HH_DBG_STREAM

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
#define HH_ARR_LEN(arr) (((arr) == NULL) ? 0 : (sizeof(arr) / sizeof((arr)[0])))

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
#define HH_STRINGIFY(x) HH__STRINGIFY(x)
// stringify booleans
// example: printf("flag: %s\n", HH_STRINGIFY_BOOL(flag));
#define HH_STRINGIFY_BOOL(x) ((x) ? "true" : "false")

// concatenate two macro arguments
#define HH_CONCATENATE(a, b) HH__CONCATENATE(a, b)

// standard assertion with variadic error message
// assertion failures are the fault of the developer

// accepts a condition expression and an optional format string & args
#define HH_ASSERT(...) HH_CONCATENATE(HH__ASSERT, HH_HAS_ARGS(__VA_ARGS__))(__VA_ARGS__)
// invariant violation assertion failures are the fault of the 'user'
#define HH_ASSERT_INVARIANT(cond) HH_ASSERT(cond, "Invariant violated: %s", HH_STRINGIFY(cond))
// use this for unreachable branches
// for example, default cases in switch blocks
#define HH_UNREACHABLE HH_ASSERT(0, "Unreachable!")

// wrappers that assert allocation success
#define hh_malloc_checked(size) HH__malloc_checked((size), __FILE__, __LINE__)
#define hh_calloc_checked(num, size) HH__calloc_checked((num), (size), __FILE__, __LINE__)

// union to easily pass around and store function pointers as data pointers
// without breaking C99 conventions
typedef union {
    const void* _ptr;
    void (*_fp)(void);
} hh_fp_wrap_t;

// accepts a function pointer and returns hh_fp_wrap_t
#define hh_fp_wrap(fp) ((hh_fp_wrap_t) { ._fp = (void(*)(void)) (fp) })
// accepts hh_fp_wrap_t* and returns a function pointer specified by fp_type
#define hh_fp_unwrap(fp_wrap, fp_type) ((fp_type) (fp_wrap ? (((hh_fp_wrap_t*) (fp_wrap))->_fp) : NULL))

// returns the number of arguments in __VA_ARGS__
// original source: https://groups.google.com/g/comp.std.c/c/d-6Mj5Lko_s?pli=1
#define HH_ARGS_LENGTH(...) HH__ARGS_LENGTH(__VA_ARGS__, HH__ARGS_LENGTH_JOIN())
// resolves to 0 when __VA_ARGS__ has exactly 1 argument, 0 otherwise
#define HH_HAS_ARGS(...) HH__HAS_ARGS(__VA_ARGS__)

// Adapted from...
// stb_ds.h - v0.67 - public domain data structures - Sean Barrett 2019

// hh_darrclear    sets array length to 0
// hh_darrfree     frees the array and sets it to NULL
// hh_darrlast     returns the last element by value
// hh_darrput      inserts a value, returns assignment result
// hh_darrputstr   pushes cstr to a char array (ensures null-termination), return pointer to start of str
// hh_darrpop      removes the last element and returns it by value
// hh_darradd      adds n zero-initialized elements to the array, returns the index to the 1st new element
// hh_darrlen      returns array length
// hh_darrcap      returns array capacity
// hh_darrswap     swaps the elements at 2 indices
// hh_darrswapdel  deletes the ith element by swapping it with the last element, then popping

#define hh_darrclear(arr)       (((arr) == NULL) ? 0 : (hh_darrheader(arr)->len = 0))
#define hh_darrfree(arr)        ((void) (((arr) == NULL) ? (void) 0 : free(hh_darrheader(arr))), (arr) = NULL)
#define hh_darrlast(arr)        ((arr)[hh_darrheader(arr)->len - 1])
#define hh_darrput(arr, val)    ((void) hh_darrgrow(arr, 1), (arr)[(hh_darrheader(arr)->len)++] = (val))
#define hh_darrputstr(arr, str) (HH__darrputstr((void**) &(arr), (str)))
#define hh_darrpop(arr)         ((arr)[--(hh_darrheader(arr)->len)])
#define hh_darradd(arr, n)      (HH__darraddn((void**) &(arr), (n), sizeof *(arr)))
#define hh_darrlen(arr)         (((arr) == NULL) ? 0 : hh_darrheader(arr)->len)
#define hh_darrcap(arr)         (((arr) == NULL) ? 0 : hh_darrheader(arr)->cap)
#define hh_darrswap(arr, i, j)  (HH__darrswap((arr), (i), (j)))
#define hh_darrswapdel(arr, i)  (HH__darrswap((arr), (i), hh_darrlen(arr) - 1), hh_darrpop(arr))

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

// parsing macros
// accepts the same optional arguments as hh_span_next,
// just parses the result afterwards
// if any stage of the parsing fails, `span` is rewound
// if the failure occurred during hh_span_next, `err` = `span`
// if it occurred during parsing, it is set to the result of hh_span_next
#define hh_span_next_lf(span, err, ...) hh_span_next_opt_lf((span), (hh_span_opt) { __VA_ARGS__ }, (err))
#define hh_span_next_ld(span, err, ...) hh_span_next_opt_ld((span), (hh_span_opt) { __VA_ARGS__ }, (err))
#define hh_span_next_zu(span, err, ...) hh_span_next_opt_zu((span), (hh_span_opt) { __VA_ARGS__ }, (err))

// contains buckets and shared fields across hh_hmap_t, hh_dict_t, etc
#define HH_HMAP_FIELDS(ty) \
    size_t bucket_count; \
    hh_##ty##_hash_f hash; \
    hh_##ty##_comp_f comp; \
    hh_##ty##_free_f free_key; \
    hh_##ty##_free_f free_val; \
    char** buckets;

// templates for custom key hashing and comparator functions
typedef size_t (*hh_hmap_hash_f)(const void* ptr);
// hh_hmap_comp_f's return value follows the same paradigm as memcmp or strcmp
//  0 indicates equality
// -1 means fst is lexographically less than snd
//  1 indicates it is greater than
typedef int    (*hh_hmap_comp_f)(const void* fst, const void* snd);
// signature for freeing both keys and values
typedef void   (*hh_hmap_free_f)(const void* ptr);

// hashmap with fixed entry sizes
typedef struct {
    size_t size_key;
    size_t size_val;
    HH_HMAP_FIELDS(hmap)
} hh_hmap_t;

// represents an element returned by hh_hmap_get and hh_dict_get
// changing the data pointed to by `val` is UB
// unless the length is preserved
typedef struct {
    size_t size_key;
    size_t size_val;
    const void* key;
    const void* val;
} hh_hmap_entry_t;
    
// insert a key-value pair into the hashmap
const void*
hh_hmap_insert(hh_hmap_t* map, const void* key, const void* val);
// returns the value associated with a key
hh_hmap_entry_t
hh_hmap_get(const hh_hmap_t* map, const void* key);
// returns the value corresponding to the given key
// NULL if the key is not a member of the map
const void*
hh_hmap_get_val(const hh_hmap_t* map, const void* key);
// remove entry corresponding to the given key
// returns truthy if an entry was removed
_Bool
hh_hmap_remove(hh_hmap_t* map, const void* key);
// iterator macro for hh_hmap
// hh_hmap_it(&map, it) printf("%d\n", *(int*) it);
#define hh_hmap_it(map, it) for(hh_hmap_entry_t it = HH__hmap_it_begin(map); it.val; HH__hmap_it_next(map, &it))
// free hh_hmap_t
void
hh_hmap_free(hh_hmap_t* map);

// functions below mirror the hh_hmap templates above
typedef size_t (*hh_dict_hash_f)(const void* ptr, size_t size_ptr);
typedef int    (*hh_dict_comp_f)(const void* fst, size_t size_fst, const void* snd, size_t size_snd);
typedef void   (*hh_dict_free_f)(const void* ptr, size_t size_ptr);

// hashmap data structure with variably-sized key-value pairs
// on initialization, only the bucket_count must be provided
// if hash and comp functions are not given, defaults are used
// standard initialization:
// hh_dict_t* hm = {32}; // sets bucket_count
// NOTE: all other fields should be 0-initialized
typedef struct {
    HH_HMAP_FIELDS(dict)
} hh_dict_t;

// dictionary entries are the same as hmap entries
typedef hh_hmap_entry_t hh_dict_entry_t;

// insert a key-value pair into the hashmap
// NOTE: more than size_val bytes MUST NOT be written into the returned value
const void*
hh_dict_insert(hh_dict_t* map, const void* key, size_t size_key, const void* val, size_t size_val);
// macro for inserting string keys
// NOTE: the key stored in the hashmap is null-terminated,
// which means hh_dict_get calls MUST also include a null-terminator 
// (assuming default hh_dict_comp_f behavior_
#define hh_dict_insert_with_cstr_key(map, key, val, size_val) hh_dict_insert(map, key, strlen(key), val, size_val)
// insert an hh_dict_entry_t element
// useful for copying from one hashmap to another
_Bool
hh_dict_insert_entry(hh_dict_t* map, const hh_dict_entry_t* entry);
// returns the key-value pair associated with a given key
// if the key does not exist in the map, the entry is 0-initialized
// NOTE: changing the underlying key & value data is a corrupting action
// if the length overruns size_key or size_val, respectively
hh_dict_entry_t
hh_dict_get(const hh_dict_t* map, const void* key, size_t size_key);
// macro for querying with cstr keys
#define hh_dict_get_with_cstr_key(map, key) hh_dict_get(map, key, strlen(key))
// returns the value corresponding to the given key
// NULL if the key is not a member of the map
const void*
hh_dict_get_val(const hh_dict_t* map, const void* key, size_t size_key);
// another helper that returns the value pointer for a cstr key, instead of the entry
#define hh_dict_get_val_with_cstr_key(map, key) hh_dict_get_val(map, key, strlen(key))
// remove entry corresponding to the given key
// returns truthy if an entry was removed
_Bool
hh_dict_remove(hh_dict_t* map, const void* key, size_t size_key);
// iterator macro for hh_dict
// hh_dict_it(&map, it) printf("%.*s", (int) it.size_key, it.key);
#define hh_dict_it(map, it) for(hh_dict_entry_t it = HH__dict_it_begin(map); it.val; HH__dict_it_next(map, &it))
// free hh_dict_t
void
hh_dict_free(hh_dict_t* map);

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
hh_args_add_command(hh_args_t* args, const char* name, const char* desc);
// parse command line arguments
// returns the final subcommand parsed, NULL on failure
const hh_args_t*
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

// a section node in an INI document tree
typedef struct HH__ini_t hh_ini_t;

// parses an INI configuration file
// return truthy on success
// otherwise, the locaiton of the parsing failure is given by err
// INI sections are hierarchical (period-delineated) i.e. [section.sub]
// whitespace around section titles is stripped
// line continuations are fully supported (subsequent whitespace is skipped)
_Bool
hh_ini_parse(hh_ini_t* ini, hh_span_t* lines, hh_span_t* err);
void
hh_ini_free(hh_ini_t* ini);
// returns a given subsection in a hierarchy
// i.e. hh_ini_query_section(ini, "section.sub")
const hh_ini_t*
hh_ini_query_section(const hh_ini_t* ini, const char* section);
// query a property in a given section hierarchy
// return NULL if the property was not parsed
const char*
hh_ini_query(const hh_ini_t* ini, const char* section, const char* key);
// query and parse a property using sscanf
// returns truthy on success
#define hh_ini_scanf(ini, section, key, fmt, ...) \
    HH__ini_scanf(ini, section, key, fmt " %n", HH_ARGS_LENGTH(__VA_ARGS__), __VA_ARGS__, &((ini)->n))
// prints the tree structure to the given stream
// NOTE: intended for debugging primarily
void
hh_ini_dump(const hh_ini_t* ini, FILE* stream);

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
// swaps two blocks of memory (possibly separated by other data)
// accepts differently-sized blocks
void
hh_memswap(char* fst, char* fst_end, char* snd, const char* snd_end);
// reverses bytes between ptr and end, in-place
void 
hh_memflip(char* ptr, const char* end);
// reverses n bytes starting at ptr, in-place
void 
hh_memflipn(char* ptr, size_t n);
//
#endif // not HH__

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

// helper for HH_STRINGIFY
#define HH__STRINGIFY(x) #x

// helper for HH_CONCATENATE
#define HH__CONCATENATE(a, b) a##b

// plain assert (if no format string is passed to HH_ASSERT)
#define HH__ASSERT0(cond) assert(cond)
// assert with message
#define HH__ASSERT1(cond, ...) do { \
    if((cond)) break; \
    HH_ERR(__VA_ARGS__); \
    assert(cond); \
} while(0)

// helpers for checked allocations
// necessary to ensure the correct line number and file are logged
void*
HH__malloc_checked(size_t size, const char* file, int line);
void*
HH__calloc_checked(size_t num, size_t size, const char* file, int line);

// initial capacity of dynamic array
#ifndef HH_DARR_INITIAL_CAPACITY
#define HH_DARR_INITIAL_CAPACITY 16
#endif // not HH_DARR_INITIAL_CAPACITY

// internal array components
typedef struct { 
    size_t len, cap, elem_size; 
} hh_darrheader_t;

// helper macros for dynamic array implementation
#define hh_darrheader(arr)  (((hh_darrheader_t*) arr) - 1)
#define hh_darrgrow(arr, n) (HH__darrgrow((void**) &(arr), (n), sizeof(*(arr))), (arr))

// helper functions for dynamic array
void 
HH__darrgrow(void** arr_ptr, size_t n, size_t elem_size);
size_t
HH__darraddn(void** arr_ptr, size_t n, size_t elem_size);

// swaps two values, used for darrswap and darrswapdel
void
HH__darrswap(void* arr, size_t i, size_t j);

// ensures null-termination and reallocation
char*
HH__darrputstr(void** arr_ptr, const char* str);

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
#endif // not HH_ARENA_DEFAULT_SIZE

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

#define HH__ARGS_LENGTH(...) HH__ARGS_LENGTH_128(__VA_ARGS__)
#define HH__ARGS_LENGTH_128(_1, _2, _3, _4, _5, _6, _7, _8, _9, \
    _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, \
    _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, \
    _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, \
    _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, \
    _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, \
    _70, _71, _72, _73, _74, _75, _76, _77, _78, _79, _80, _81, \
    _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, \
    _94, _95, _96, _97, _98, _99, _100, _101, _102, _103, _104, \
    _105, _106, _107, _108, _109, _110, _111, _112, _113, _114, \
    _115, _116, _117, _118, _119, _120, _121, _122, _123, _124, \
    _125, _126, _127, N, ...) N

#define HH__ARGS_LENGTH_JOIN() \
    127, 126, 125, 124, 123, 122, 121, 120, \
    119, 118, 117, 116, 115, 114, 113, 112, \
    111, 110, 109, 108, 107, 106, 105, 104, \
    103, 102, 101, 100, 99, 98, 97, 96, 95, \
    94, 93, 92, 91, 90, 89, 88, 87, 86, 85, \
    84, 83, 82, 81, 80, 79, 78, 77, 76, 75, \
    74, 73, 72, 71, 70, 69, 68, 67, 66, 65, \
    64, 63, 62, 61, 60, 59, 58, 57, 56, 55, \
    54, 53, 52, 51, 50, 49, 48, 47, 46, 45, \
    44, 43, 42, 41, 40, 39, 38, 37, 36, 35, \
    34, 33, 32, 31, 30, 29, 28, 27, 26, 25, \
    24, 23, 22, 21, 20, 19, 18, 17, 16, 15, \
    14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define HH__HAS_ARGS(...) HH__ARGS_LENGTH_128(__VA_ARGS__, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, )

// if the following is defined
// hh_span_next_lf, hh_span_next_ld, hh_span_next_zu, etc
// will return huge values on failure.
// this is included solely as a diagnostic
// #define HH_SPAN_RETURN_ODDITY_ON_PARSE_FAILURE

// underlying function behind hh_span_next
hh_span_t
hh_span_next_opt(hh_span_t* s, hh_span_opt opt);
// underlying functions behind hh_span_next_lf, hh_span_next_ld, hh_span_next_zu, etc
double
hh_span_next_opt_lf(hh_span_t* span, hh_span_opt opt, hh_span_t* err);
long
hh_span_next_opt_ld(hh_span_t* span, hh_span_opt opt, hh_span_t* err);
size_t
hh_span_next_opt_zu(hh_span_t* span, hh_span_opt opt, hh_span_t* err);

// NetBSD: getline.c,v 1.2 2014/09/16 17:23:50 christos Exp
ptrdiff_t // NO PREFIX STRIPPING
hh_getdelim(char** buf, size_t* bufsiz, int delimiter, FILE* fp);
ptrdiff_t // NO PREFIX STRIPPING
hh_getline(char** buf, size_t* bufsiz, FILE* fp);

#undef HH_HMAP_FIELDS

// the default number of buckets to be used by hh_hmap and hh_dict
#ifndef HH_BUCKET_COUNT
#define HH_BUCKET_COUNT 16
#endif // not HH_BUCKET_COUNT

hh_hmap_entry_t
HH__hmap_it_begin(const hh_hmap_t* map);
void
HH__hmap_it_next(const hh_hmap_t* map, hh_hmap_entry_t* entry);

hh_dict_entry_t
HH__dict_it_begin(const hh_dict_t* map);
void
HH__dict_it_next(const hh_dict_t* map, hh_dict_entry_t* entry);

// defines indentation of the usage tree (must be >2)
#ifndef HH_ARGS_USAGE_INDENT
#define HH_ARGS_USAGE_INDENT 4
#endif // not HH_ARGS_USAGE_INDENT

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
        HH__ARGS_ERR_COMMAND_MISSING,
        HH__ARGS_ERR_COMMAND_DOESNT_EXIST,
        HH__ARGS_ERR_FLAG_MISSING_VALUE,
        HH__ARGS_ERR_FLAG_INVALID_VALUE,
        HH__ARGS_ERR_FLAG_DUPLICATE,
        HH__ARGS_ERR_REQUIRED_FLAG_MISSING,
        HH__ARGS_ERR_FLAG_INCOMPATIBLE_WITH_COMMAND
    } type;
    const struct HH__args_entry* entry;
    const char* extra;
};

struct HH__args_data {
    hh_arena entries;
    const struct HH__args_entry* entry_help;
    hh_dict_t flags;
    hh_dict_t flags_long;
    const hh_args_t* deepest_parsed;
    struct HH__args_error error;
};

// DONE: paths shouldn't necessary have to exist to be valid
// if they can be parsed by hh_path_alloc, then it's okay
// consider splitting the flag types into HH_FLAG_PATH
// and HH_PATH_EXISTS

// TODO: Below is the bucket list for hh_args_t, ordered by precedence
// DONE: * [--help, -h] to print context-specific help menus
// DONE: * required arguments
// DONE: ** error handling when missing required args
// * repeated arguments
// DONE: * error when passing flags that belong to different commands
struct HH__args_t {
    const char* name;
    const char* desc;
    _Bool parsed;
    uintptr_t* entries;
    hh_args_t* children;
    hh_args_t* parent;
    struct HH__args_data* data;
};

const void*
HH__args_add_flag(hh_args_t* args, hh_flag_type type, hh_flag_opt opt);

#ifndef HH_INI_INDENT
#define HH_INI_INDENT 2
#endif // not HH_INI_INDENT

struct HH__ini_t {
    hh_dict_t sections;
    hh_dict_t props;
    int n;
};

// implementation for hh_ini_scanf
// using HH_ARGS_LENGTH and the %n format specifier, 
// this function can check
// * that every arg was parsed
// * that the entire property value was parsed
_Bool
HH__ini_scanf(const hh_ini_t* ini, const char* section, const char* key, const char* fmt, int n, ...);
//
#endif // HH__

#ifdef HH_IMPLEMENTATION
// implementation-exclusive includes
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef HH_SPAN_RETURN_ODDITY_ON_PARSE_FAILURE
#include <math.h>
#include <float.h>
#include <limits.h>
#endif // HH_SPAN_RETURN_ODDITY_ON_PARSE_FAILURE

// platform-dependent includes
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif // _WIN32

void*
HH__malloc_checked(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if(ptr == NULL) {
        fprintf(HH_ERR_STREAM, "ERROR [%s:%d]: "
            "Failed to allocate %llu bytes\n", 
            file, line, (unsigned long long) size);
        abort();
    }
    return ptr;
}

void*
HH__calloc_checked(size_t num, size_t size, const char* file, int line) {
    void* ptr = calloc(num, size);
    if(ptr == NULL) {
        fprintf(HH_ERR_STREAM, "ERROR [%s:%d]: "
            "Failed to allocate %llu objects [%llu bytes]\n", 
            file, line, (unsigned long long) num, (unsigned long long) (size * num));
        abort();
    }
    return ptr;
}

void 
HH__darrgrow(void** arr_ptr, size_t n, size_t elem_size) {
    HH_ASSERT_INVARIANT(arr_ptr != NULL);
    HH_ASSERT_INVARIANT(elem_size > 0);
    hh_darrheader_t* arr_hdr;
    if(*arr_ptr == NULL) {
        arr_hdr = calloc(1, sizeof(hh_darrheader_t) + elem_size * HH_MAX(n, HH_DARR_INITIAL_CAPACITY));
        HH_ASSERT(arr_hdr != NULL, "HH__darrgrow failed to allocate array");
        arr_hdr->len = 0;
        arr_hdr->cap = HH_MAX(n, HH_DARR_INITIAL_CAPACITY);
        arr_hdr->elem_size = elem_size;
        *arr_ptr = (void*) (arr_hdr + 1);
        return;
    }
    arr_hdr = hh_darrheader(*arr_ptr);
    if(arr_hdr->len + n < arr_hdr->cap) return;
    while(arr_hdr->len + n >= arr_hdr->cap) arr_hdr->cap *= 2;
    arr_hdr = realloc(arr_hdr, sizeof(hh_darrheader_t) + arr_hdr->cap * arr_hdr->elem_size);
    HH_ASSERT(arr_hdr != NULL, "HH__darrgrow failed to allocate array");
    *arr_ptr = (void*) (arr_hdr + 1);
}

size_t
HH__darraddn(void** arr_ptr, size_t n, size_t elem_size) {
    HH__darrgrow(arr_ptr, n, elem_size);
    size_t len = hh_darrlen(*arr_ptr);
    if(n > 0) {
        memset((char*) (*arr_ptr) + len * elem_size, 0, elem_size * n);
        hh_darrheader(*arr_ptr)->len = len + n;
    }
    return len;
}

char*
HH__darrputstr(void** arr_ptr, const char* str) {
    HH_ASSERT_INVARIANT(arr_ptr != NULL);
    HH_ASSERT_INVARIANT(arr_ptr[0] == NULL || (arr_ptr[0] != NULL && hh_darrheader(arr_ptr[0])->elem_size == 1));
    // determine if the array currently ends in a null terminator (n == 0)
    size_t n = 0;
    if(hh_darrlen(*arr_ptr) == 0) n = 1;
    else if((hh_darrlen(*arr_ptr) != 0 && (((char**) arr_ptr)[0] + hh_darrlen(*arr_ptr) - 1)[0] != '\0')) n = 1;
    size_t idx = HH__darraddn(arr_ptr, ((str == NULL) ? 0 : strlen(str)) + n, 1);
    HH_ASSERT((n == 0 && idx > 0) || n > 0);
    if(str == NULL && n > 0) ((char**) arr_ptr)[0][idx] = '\0';
    else strcpy(((char**) arr_ptr)[0] + (idx -= (n == 0)), (str));
    return ((char**) arr_ptr)[0] + idx;
}

void
HH__darrswap(void* arr, size_t i, size_t j) {
    HH_ASSERT_INVARIANT(arr != NULL);
    HH_ASSERT_INVARIANT(hh_darrlen(arr) > 0);
    HH_ASSERT_INVARIANT(i < hh_darrlen(arr) && j < hh_darrlen(arr));
    if(i == j) return;
    size_t elem_size = hh_darrheader(arr)->elem_size;
    char* elem_i = ((char*) arr) + i * elem_size;
    char* elem_j = ((char*) arr) + j * elem_size;
    for(size_t k = 0; k < elem_size; ++k) {
        elem_i[k] = elem_i[k] ^ elem_j[k];
        elem_j[k] = elem_i[k] ^ elem_j[k];
        elem_i[k] = elem_i[k] ^ elem_j[k];
    }
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
    char* path = NULL;
#ifdef _WIN32
    char* raw_abs = NULL;
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
    hh_darrputstr(path, raw_abs);
    free(raw_abs);
#else // _WIN32
    char* cmd = NULL;
    hh_darrputstr(cmd, "readlink -m ");
    hh_darrputstr(cmd, raw);
    FILE *fp = popen(cmd, "r");
    if(fp == NULL) {
        perror("popen");
        return NULL;
    }
    hh_darrfree(cmd);
    int ch;
    while((ch = getc(fp)) != EOF && ch != '\n') 
        hh_darrput(path, (char) ch);
    hh_darrput(path, '\0');
    pclose(fp);
#endif // not _WIN32
    // length of root path is platform-dependent
#ifdef _WIN32
#define HH__PATH_ROOT_LEN 3
#else
#define HH__PATH_ROOT_LEN 1
#endif
    if(path == NULL || hh_darrlen(path) <= HH__PATH_ROOT_LEN) return NULL;
    for(size_t i = 0; path[i]; i++) if(path[i] == '\\') path[i] = '/';
    size_t len = hh_darrlen(path);
    if(len > (HH__PATH_ROOT_LEN + 1) && path[len - 2] == '/' && path[len - 1] == '\0') {
        path[len - 2] = '\0';
        hh_darrheader(path)->len -= 1;
    }
    return path;
#undef HH__PATH_ROOT_LEN
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

#define HH__SPAN_PROLOGUE(err_ret) \
    if(err != NULL && err->ptr != NULL) return (err_ret); \
    hh_span_t prev = *span; \
    hh_span_t token = hh_span_next_opt(span, opt); \
    if(token.ptr == NULL) { \
        *err = prev; \
        return (err_ret); \
    } \
    HH_ASSERT_INVARIANT(token.end != NULL);

#define HH__SPAN_EPILOGUE(err_ret, err_cond) \
    if(err_cond) { \
        *span = prev; \
        *err = token; \
        return (err_ret); \
    }

double
hh_span_next_opt_lf(hh_span_t* span, hh_span_opt opt, hh_span_t* err) {
#ifdef HH_SPAN_RETURN_ODDITY_ON_PARSE_FAILURE
#define HH__SPAN_ERROR HUGE_VAL
#else
#define HH__SPAN_ERROR 0.0
#endif // HH_SPAN_RETURN_ODDITY_ON_PARSE_FAILURE
    HH__SPAN_PROLOGUE(HH__SPAN_ERROR);
    char* end;
    double val = strtod(token.ptr, &end);
    HH__SPAN_EPILOGUE(HH__SPAN_ERROR, end == token.ptr || end != token.end || errno == ERANGE);
    return val;
#undef HH__SPAN_ERROR
}

long
hh_span_next_opt_ld(hh_span_t* span, hh_span_opt opt, hh_span_t* err) {
#ifdef HH_SPAN_RETURN_ODDITY_ON_PARSE_FAILURE
#define HH__SPAN_ERROR LONG_MAX
#else
#define HH__SPAN_ERROR 0
#endif // HH_SPAN_RETURN_ODDITY_ON_PARSE_FAILURE
    HH__SPAN_PROLOGUE(HH__SPAN_ERROR);
    char* end;
    long val = strtol(token.ptr, &end, 10);
    HH__SPAN_EPILOGUE(HH__SPAN_ERROR, end == token.ptr || end != token.end || errno == ERANGE);
    return val;
#undef HH__SPAN_ERROR
}

size_t
hh_span_next_opt_zu(hh_span_t* span, hh_span_opt opt, hh_span_t* err) {
#ifdef HH_SPAN_RETURN_ODDITY_ON_PARSE_FAILURE
#define HH__SPAN_ERROR ULONG_MAX
#else
#define HH__SPAN_ERROR 0
#endif // HH_SPAN_RETURN_ODDITY_ON_PARSE_FAILURE
    HH__SPAN_PROLOGUE(HH__SPAN_ERROR);
    char* end;
    size_t val = strtoul(token.ptr, &end, 10);
    HH__SPAN_EPILOGUE(HH__SPAN_ERROR, end == token.ptr || end != token.end || errno == ERANGE);
    return val;
#undef HH__SPAN_ERROR
}

#undef HH__SPAN_PROLOGUE
#undef HH__SPAN_EPILOGUE

// adapted from the following link
// https://gist.github.com/MohamedTaha98/ccdf734f13299efb73ff0b12f7ce429f
// thanks to MohamedTaha98
static size_t
HH__hash_djb2(const void* ptr, size_t size_ptr) {
    size_t hash = 5381;
    for(size_t i = 0; i < size_ptr; ++i) hash = ((hash << 5) + hash) + (size_t) ((char*) ptr)[i];
    return hash;
}

static size_t
HH__hmap_hash_generic(const hh_hmap_t* map, const void* ptr) {
    return ((map->hash == NULL) ? 
        HH__hash_djb2(ptr, map->size_key) : 
        (map->hash)(ptr)) % map->bucket_count;
}

static int
HH__hmap_comp_generic(const hh_hmap_t* map, const void* fst, const void* snd) {
    if(map->comp != NULL) return (map->comp)(fst, snd);
    return memcmp(fst, snd, map->size_key);
}

const void*
hh_hmap_insert(hh_hmap_t* map, const void* key, const void* val) {
    HH_ASSERT_INVARIANT(map != NULL);
    HH_ASSERT_INVARIANT(key != NULL);
    // initialize map
    if(map->buckets == NULL) {
        HH_ASSERT_INVARIANT(map->size_key > 0);
        if(map->bucket_count == 0) map->bucket_count = HH_BUCKET_COUNT;
        map->buckets = calloc(map->bucket_count, sizeof(char*));
        if(map->buckets == NULL) return NULL;
    }
    hh_hmap_remove(map, key);
    // perform insertion
    size_t idx, len;
    idx = HH__hmap_hash_generic(map, key);
    len = hh_darrlen(map->buckets[idx]);
    hh_darradd(map->buckets[idx], map->size_key + map->size_val);
    // copy over entry
    char* key_start = map->buckets[idx] + len;
    memcpy(key_start, key, map->size_key);
    char* val_start = key_start + map->size_key;
    if(val == NULL) memset(val_start, 0, map->size_val);
    else memcpy(val_start, val, map->size_val);
    return val_start;
}

hh_hmap_entry_t
hh_hmap_get(const hh_hmap_t* map, const void* key) {
    HH_ASSERT_INVARIANT(map != NULL);
    HH_ASSERT_INVARIANT(key != NULL);
    HH_ASSERT_INVARIANT(map->size_key > 0);
    if(map->buckets == NULL) goto failure;
    // get correct bucket
    size_t idx = HH__hmap_hash_generic(map, key);
    // step through the bucket
    hh_hmap_entry_t entry;
    for(size_t i = 0; i < hh_darrlen(map->buckets[idx]);) {
        entry.key = map->buckets[idx] + i; i += map->size_key;
        entry.val = map->buckets[idx] + i; i += map->size_val;
        // return if key was found
        if(HH__hmap_comp_generic(map, key, entry.key) == 0) {
            entry.size_key = map->size_key;
            entry.size_val = map->size_val;
            return entry;
        }
    }
failure:
    return (hh_hmap_entry_t) {0};
}

const void*
hh_hmap_get_val(const hh_hmap_t* map, const void* key) {
    hh_hmap_entry_t entry = hh_hmap_get(map, key);
    return entry.val;
}

_Bool
hh_hmap_remove(hh_hmap_t* map, const void* key) {
    HH_ASSERT_INVARIANT(map != NULL);
    HH_ASSERT_INVARIANT(key != NULL);
    HH_ASSERT_INVARIANT(map->size_key > 0);
    // get corresponding entry
    char* val = (char*) hh_hmap_get_val(map, key);
    if(val == NULL) return 0;
    // recompute bucket
    size_t idx = HH__hmap_hash_generic(map, key);
    char* bucket_begin = map->buckets[idx];
    char* bucket_end = bucket_begin + hh_darrlen(bucket_begin);
    // entry bounds
    char* entry_begin = val - map->size_key;
    const char* entry_end = val + map->size_val;
    // remaining bytes to slide over the current entry
    memmove(entry_begin, entry_end, (size_t) (bucket_end - entry_end));
    // update hh_darrheader_t length to reflect changes
    hh_darrheader(bucket_begin)->len--;
    return 1;
}

hh_hmap_entry_t
HH__hmap_it_begin(const hh_hmap_t* map) {
    HH_ASSERT_INVARIANT(map != NULL);
    if(map->buckets == NULL) goto finish;
    // scan all buckets until a non-empty one is found
    for(size_t idx = 0; idx < map->bucket_count; ++idx) {
        if(hh_darrlen(map->buckets[idx]) == 0) continue; 
        return (hh_hmap_entry_t) { 
            .size_key = map->size_key, 
            .size_val = map->size_val, 
            .key = map->buckets[idx], 
            .val = map->buckets[idx] + map->size_key 
        };
    }
finish:
    // bucket is empty
    return (hh_hmap_entry_t) {0};
}

void
HH__hmap_it_next(const hh_hmap_t* map, hh_hmap_entry_t* entry) {
    HH_ASSERT_INVARIANT(map != NULL);
    const char* entry_begin;
    size_t idx;
    // compute the bucket index of the entry
    entry_begin = entry->key;
    idx = HH__hmap_hash_generic(map, entry_begin);
    // look for the next entry in the same bucket
    entry_begin += map->size_key + map->size_val;
    if(entry_begin < map->buckets[idx] + hh_darrlen(map->buckets[idx])) {
        entry->key = entry_begin;
        entry->val = entry_begin + map->size_key;
        return;
    }
    // scan remaining buckets if we didn't find another element in the previous one
    for(++idx; idx < map->bucket_count; ++idx) {
        if(hh_darrlen(map->buckets[idx]) == 0) continue;
        entry->key = map->buckets[idx];
        entry->val = map->buckets[idx] + map->size_key;
        return;
    }
    // end of iteration
    memset(entry, 0, sizeof(hh_hmap_entry_t));
}

void
hh_hmap_free(hh_hmap_t* map) {
    HH_ASSERT_INVARIANT(map != NULL);
    const char* key;
    const char* val;
    if(map->buckets == NULL) return;
    for(size_t i = 0; i < map->bucket_count; ++i) {
        for(size_t j = 0; j < hh_darrlen(map->buckets[i]);) {
            key = map->buckets[i] + j; j += map->size_key;
            val = map->buckets[i] + j; j += map->size_val;
            if(map->free_key) (map->free_key)(key);
            if(map->free_val) (map->free_val)(val);
        }
        hh_darrfree(map->buckets[i]);
    }
    free(map->buckets);
}

static size_t
HH__dict_hash_generic(const hh_dict_t* map, const void* ptr, size_t size_ptr) {
    return ((map->hash == NULL) ? 
        HH__hash_djb2(ptr, size_ptr) : 
        (map->hash)(ptr, size_ptr)) % map->bucket_count;
}

static int
HH__dict_comp_generic(const hh_dict_t* map, const void* fst, size_t size_fst, const void* snd, size_t size_snd) {
    if(map->comp != NULL) return (map->comp)(fst, size_fst, snd, size_snd);
    int result = memcmp(fst, snd, HH_MIN(size_fst, size_snd));
    if(result != 0) return result;
    if(size_fst < size_snd) return -1;
    if(size_fst > size_snd) return 1;
    return 0;
}

const void*
hh_dict_insert(hh_dict_t* map, const void* key, size_t size_key, const void* val, size_t size_val) {
    HH_ASSERT_INVARIANT(map != NULL);
    HH_ASSERT_INVARIANT(key != NULL);
    HH_ASSERT_INVARIANT(size_key > 0);
    // initialize map
    if(map->buckets == NULL) {
        if(map->bucket_count == 0) map->bucket_count = HH_BUCKET_COUNT;
        map->buckets = calloc(map->bucket_count, sizeof(char*));
        if(map->buckets == NULL) return NULL;
        for(size_t i = 0; i < map->bucket_count; ++i) 
            hh_darradd(map->buckets[i], sizeof(size_t) * 2);
    }
    hh_dict_remove(map, key, size_key);
    // perform insertion
    size_t idx, len;
    idx = HH__dict_hash_generic(map, key, size_key);
    len = hh_darrlen(map->buckets[idx]);
    hh_darradd(map->buckets[idx], size_key + size_val + sizeof(size_t) * 2);
    // update entry sizes
    size_t* meta = (((size_t*) (map->buckets[idx] + len)) - 2);
    HH_ASSERT(meta[0] == 0 && meta[1] == 0);
    *(meta++) = size_key;
    *(meta++) = size_val;
    // copy over entry
    memcpy(meta, key, size_key);
    char* val_start = ((char*) meta) + size_key;
    if(val == NULL) memset(val_start, 0, size_val);
    else memcpy(val_start, val, size_val);
    return val_start;
}

_Bool
hh_dict_insert_entry(hh_dict_t* map, const hh_dict_entry_t* entry) {
    return hh_dict_insert(map, entry->key, entry->size_key, entry->val, entry->size_val);
}

hh_dict_entry_t
hh_dict_get(const hh_dict_t* map, const void* key, size_t size_key) {
    HH_ASSERT_INVARIANT(map != NULL);
    HH_ASSERT_INVARIANT(key != NULL);
    HH_ASSERT_INVARIANT(size_key > 0);
    if(map->buckets == NULL) return (hh_dict_entry_t) {0};
    // get correct bucket
    size_t idx = HH__dict_hash_generic(map, key, size_key);
    // step through the bucket
    hh_dict_entry_t entry;
    for(size_t i = 0; i < hh_darrlen(map->buckets[idx]);) {
        entry.size_key = *((size_t*) (map->buckets[idx] + i)); i += sizeof(size_t);
        entry.size_val = *((size_t*) (map->buckets[idx] + i)); i += sizeof(size_t);
        entry.key = map->buckets[idx] + i; i += entry.size_key;
        entry.val = map->buckets[idx] + i; i += entry.size_val;
        // return if key was found
        if(entry.size_key == 0 && entry.size_val == 0) goto failure;
        if(HH__dict_comp_generic(map, key, size_key, entry.key, entry.size_key) == 0) 
            return entry;
    }
failure:
    return (hh_dict_entry_t) {0};
}

const void*
hh_dict_get_val(const hh_dict_t* map, const void* key, size_t size_key) {
    return hh_dict_get(map, key, size_key).val;
}

_Bool
hh_dict_remove(hh_dict_t* map, const void* key, size_t size_key) {
    HH_ASSERT_INVARIANT(map != NULL);
    HH_ASSERT_INVARIANT(key != NULL);
    HH_ASSERT_INVARIANT(size_key > 0);
    // get corresponding entry
    hh_dict_entry_t entry = hh_dict_get(map, key, size_key);
    if(entry.val == NULL) return 0;
    // recompute bucket
    size_t idx = HH__dict_hash_generic(map, key, size_key);
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

static void
HH__dict_it_helper(hh_dict_entry_t* entry, const char* entry_begin) {
    entry->size_key = ((size_t*) entry_begin)[0];
    entry->size_val = ((size_t*) entry_begin)[1];
    entry->key = (const void*) (((size_t*) entry_begin) + 2);
    entry->val = (const char*) entry->key + entry->size_key;
}

hh_dict_entry_t
HH__dict_it_begin(const hh_dict_t* map) {
    HH_ASSERT_INVARIANT(map != NULL);
    if(map->buckets == NULL) goto finish;
    // scan all buckets until a non-empty one is found
    hh_dict_entry_t entry;
    for(size_t idx = 0; idx < map->bucket_count; ++idx) {
        // a non-empty bucket is longer than the terminating 0, 0
        if(hh_darrlen(map->buckets[idx]) > sizeof(size_t) * 2) {
            HH__dict_it_helper(&entry, map->buckets[idx]);
            return entry;
        }
    }
finish:
    // bucket is empty
    return (hh_dict_entry_t) {0};
}

void
HH__dict_it_next(const hh_dict_t* map, hh_dict_entry_t* entry) {
    HH_ASSERT_INVARIANT(map != NULL);
    char* entry_begin;
    size_t idx;
    // compute the bucket index of the entry
    idx = HH__dict_hash_generic(map, entry->key, entry->size_key);
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
    memset(entry, 0, sizeof(hh_dict_entry_t));
    return;
    // continued iteration
found:
    HH__dict_it_helper(entry, entry_begin);
    return;
}

void
hh_dict_free(hh_dict_t* map) {
    HH_ASSERT_INVARIANT(map != NULL);
    hh_dict_entry_t entry;
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
HH__flag_value_name(hh_flag_opt opt, const hh_flag_type* type) {
    if(type == NULL) return NULL;
    HH_ASSERT(opt.name == NULL || type != HH_FLAG_BOOL);
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

// NOTE: I have no excuse for this
#define HH__FLAG_FMT "%s-%s%.*s%s%s%s%s%s%s"
#define HH__FLAG_FMT_ARGS(opt, type) \
        ((opt).required) ? "" : "[", \
        "", \
        ((opt).flag == '\0') ? 0 : 1, \
        ((opt).flag == '\0') ? "" : &(opt).flag, \
        ((opt).flag == '\0') ? "-" : (((opt).flag_long == NULL) ? "" : ", --"), \
        ((opt).flag_long == NULL) ? "" : (opt).flag_long, \
        ((type) == NULL || *((hh_flag_type*) (type)) == HH_FLAG_BOOL) ? "" : " <", \
        ((type) == NULL || *((hh_flag_type*) (type)) == HH_FLAG_BOOL) ? "" : HH__flag_value_name(opt, type), \
        ((type) == NULL || *((hh_flag_type*) (type)) == HH_FLAG_BOOL) ? "" : ">", \
        ((opt).required) ? "" : "]"
#define HH__FLAG_FMT_ARGS_SHORT(opt, type) \
        ((opt).required) ? "" : "[", \
        ((opt).flag == '\0') ? "-" : "", \
        ((opt).flag == '\0') ? ((int) strlen((opt).flag_long)) : 1, \
        ((opt).flag == '\0') ? (opt).flag_long : &(opt).flag, \
        "", \
        "", \
        ((type) == NULL || *((hh_flag_type*) (type)) == HH_FLAG_BOOL) ? "" : " <", \
        ((type) == NULL || *((hh_flag_type*) (type)) == HH_FLAG_BOOL) ? "" : HH__flag_value_name(opt, type), \
        ((type) == NULL || *((hh_flag_type*) (type)) == HH_FLAG_BOOL) ? "" : ">", \
        ((opt).required) ? "" : "]"

#define HH__ARGS_INVALID "Invalid hh_args_t configuration. "

struct HH__args_entry*
hh_args_data_add_flag(struct HH__args_data* data, hh_flag_type type, hh_flag_opt opt) {
    // fail assert early if no flag is provided
    HH_ASSERT(opt.flag != '\0' || opt.flag_long != NULL, 
        HH__ARGS_INVALID "Either short or long flag must be set");
    // ensure flag and flag_long don't already exist
    HH_ASSERT(opt.flag != '\0' && !hh_dict_get_val(&data->flags, &opt.flag, 1), 
        HH__ARGS_INVALID "Flag '" HH__FLAG_FMT "' already added", 
        HH__FLAG_FMT_ARGS(opt, NULL));
    HH_ASSERT(opt.flag_long != NULL && !hh_dict_get_val_with_cstr_key(&data->flags_long, opt.flag_long),
        HH__ARGS_INVALID "Long flag '" HH__FLAG_FMT "' already added", 
        HH__FLAG_FMT_ARGS(opt, NULL));
    // provided 'name' to boolean
    HH_ASSERT(type == HH_FLAG_BOOL && opt.name == NULL, 
        HH__ARGS_INVALID "Provided value name to boolean flag '" HH__FLAG_FMT "'", 
        HH__FLAG_FMT_ARGS(opt, NULL));
    HH_ASSERT(type == HH_FLAG_BOOL && !opt.required, 
        HH__ARGS_INVALID "Boolean flag '" HH__FLAG_FMT "' can't be required: ", 
        HH__FLAG_FMT_ARGS(opt, NULL));
    // allocate
    struct HH__args_entry* entry = hh_arena_alloc(&data->entries, sizeof(*entry));
    if(entry == NULL) return NULL;
    // insert into hashmaps
    HH_ASSERT(opt.flag != '\0' && hh_dict_insert(&data->flags, &opt.flag, 1, &entry, sizeof(uintptr_t)), 
        HH__ARGS_INVALID "Failed to add flag '" HH__FLAG_FMT "' to hh_args_t", 
        HH__FLAG_FMT_ARGS(opt, NULL));
    HH_ASSERT(opt.flag_long != NULL && hh_dict_insert_with_cstr_key(&data->flags_long, opt.flag_long, &entry, sizeof(uintptr_t)),
        HH__ARGS_INVALID "Failed to add long flag '" HH__FLAG_FMT "' to hh_args_t: ", 
        HH__FLAG_FMT_ARGS(opt, NULL));
    // assign and return
    entry->flag = opt;
    entry->type = type;
    return entry;
}

static struct HH__args_data*
HH__args_data_init(void) {
    struct HH__args_data* data = calloc(1, sizeof(*data));
    HH_ASSERT(data != NULL, HH__ARGS_INVALID "Failed to allocate");
    // add [-h, --help] flag
    static const hh_flag_opt opt = { 
        .flag = 'h', 
        .flag_long = "help", 
        .desc = "show this help menu" 
    };
    data->entry_help = hh_args_data_add_flag(data, HH_FLAG_BOOL, opt);
    HH_ASSERT(data->entry_help != NULL, "Failed to add flag '" HH__FLAG_FMT "'", HH__FLAG_FMT_ARGS(opt, NULL));
    return data;
}

const void*
HH__args_add_flag(hh_args_t* args, hh_flag_type type, hh_flag_opt opt) {
    HH_ASSERT_INVARIANT(args != NULL);
    // initialize
    if(args->data == NULL) args->data = HH__args_data_init();
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
hh_args_add_command(hh_args_t* args, const char* name, const char* desc) {
    HH_ASSERT_INVARIANT(name != NULL);
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

#undef HH__ARGS_INVALID

#define HH__ARGS_ERR_SET(args, ...) \
    args->data->error = ((struct HH__args_error) { __VA_ARGS__ });

static hh_args_t*
HH__args_root(hh_args_t* args) {
    hh_args_t* root = args;
    while(root->parent != NULL) root = root->parent;
    return root;
}

static struct HH__args_entry*
HH__args_parse_entry_inclusive(hh_args_t* args, const char* argi, const char** val) {
    HH_ASSERT(args != NULL);
    if(argi == NULL) return NULL;
    size_t len = strlen(argi);
    const char* argi_split = NULL;
    const void* ptr;
    if(len > 2 && hh_has_prefix(argi, "--")) {
        // check if there is an equals
        if((argi_split = strchr(argi, '=')))
            ptr = hh_dict_get_val(&args->data->flags_long, argi + 2, (size_t) (argi_split - argi - 2));
        else ptr = hh_dict_get_val_with_cstr_key(&args->data->flags_long, argi + 2);
    } else if(len > 1 && argi[0] == '-') {
        if(len > 2) argi_split = argi + 1;
        ptr = hh_dict_get_val(&args->data->flags, argi + 1, 1);
    } else return 0;
    if(ptr == NULL) return 0;
    // check if the value is part of this argument
    if(argi_split != NULL && val != NULL) val[0] = argi_split + 1;
    return ((struct HH__args_entry**) ptr)[0];
}

static struct HH__args_entry*
HH__args_parse_entry(hh_args_t* args, const char* argi, const char** val) {
    struct HH__args_entry* query = HH__args_parse_entry_inclusive(args, argi, val);
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
HH__args_parse_inner(hh_args_t* args, int argc, char* argv[]) {
    // verify that we received enough commands/subcommands
    if(argc == 0 && hh_darrlen(args->children) > 0) {
        HH__ARGS_ERR_SET(args, .type = HH__ARGS_ERR_COMMAND_MISSING);
        return 0;
    }
    // invoke subcommands
    _Bool found = !hh_darrlen(args->children);
    for(size_t i = 0; i < hh_darrlen(args->children); ++i) {
        if(strcmp(argv[0], args->children[i].name) != 0) continue;
        // set the child node as the latest parsed
        args->data->deepest_parsed = &args->children[i];
        args->children[i].parsed = 1;
        if(!HH__args_parse_inner(&args->children[i], argc - 1, argv + 1)) return 0;
        found = 1;
    }
    // error if we failed to find one
    struct HH__args_entry* entry;
    if(!found) {
        entry = HH__args_parse_entry(HH__args_root(args), argv[0], NULL);
        HH__ARGS_ERR_SET(args, 
            .type = HH__ARGS_ERR_COMMAND_DOESNT_EXIST,
            .entry = entry,
            .extra = argv[0]);
        return 0;
    }
    // attempt to parse all arguments from argv[1] onwards
    const char* ptr;
    char* end;
    for(ptr = NULL; argv[0]; ++argv, ptr = NULL, entry = NULL) {
        entry = HH__args_parse_entry(args, argv[0], &ptr);
        if(entry == NULL) continue;
        if(entry->set) {
            HH__ARGS_ERR_SET(args,
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
                HH__ARGS_ERR_SET(args, 
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
            HH__ARGS_ERR_SET(args,
                .type = HH__ARGS_ERR_REQUIRED_FLAG_MISSING,
                .entry = entry);
            return 0;
        }
    }
    return 1;
    // set error for an incorrectly parsed flag value
invalid:
    HH__ARGS_ERR_SET(args, 
        .type = HH__ARGS_ERR_FLAG_INVALID_VALUE,
        .entry = entry,
        .extra = ptr);
    return 0;
}

const hh_args_t*
hh_args_parse(hh_args_t* args, FILE* stream, int argc, char* argv[]) {
    HH_ASSERT_INVARIANT(args != NULL);
    HH_ASSERT_INVARIANT(argc > 0);
    HH_ASSERT_INVARIANT(argv[argc] == NULL);
    HH_ASSERT_INVARIANT(args->parent == NULL);
    args->data->deepest_parsed = args;
    _Bool result = HH__args_parse_inner(args, argc - 1, argv + 1);
    // disregard error if [-h, --help] flag was passed
    for(int i = 0; i < argc; ++i) {
        if(args->data->entry_help != HH__args_parse_entry_inclusive(args, argv[i], NULL)) continue;
        hh_args_print_usage(args, stream, argc, argv);
        return args->data->deepest_parsed;
    }
    if(!result) return NULL;
    // ensure no flags corresponding to unused commands are passed
    const struct HH__args_entry* entry;
    for(int i = 0; i < argc; ++i) {
        entry = HH__args_parse_entry_inclusive(args, argv[i], NULL);
        if(entry == NULL) continue;
        if(!entry->set) {
            HH__ARGS_ERR_SET(args,
                .type = HH__ARGS_ERR_FLAG_INCOMPATIBLE_WITH_COMMAND,
                .entry = entry);
            return NULL;
        }
    }
    return args->data->deepest_parsed;
}

#undef HH__ARGS_ERR_SET

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
        hh_dict_free(&args->data->flags);
        hh_dict_free(&args->data->flags_long);
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
    if(args->data->error.entry != NULL) fprintf(stream, HH__FLAG_FMT, HH__FLAG_FMT_ARGS(args->data->error.entry->flag));
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
HH__args_print_error_helper(const hh_args_t* origin, FILE* stream) {
    size_t len = hh_darrlen(origin->children);
    if(origin->parent == NULL) {
        fprintf(stream, "command");
    } else {
        HH_ASSERT(origin->name != NULL);
        fprintf(stream, "subcommand for '%s'", origin->name);
    }
    fputc(' ', stream);
    HH_ASSERT(len > 0);
    fprintf(stream, "[must be one of: ");
    for(size_t i = 0; i < len; ++i) {
        fprintf(stream, "%s%s", origin->children[i].name, (i + 1 < len) ? ", " : "");
    }
    fprintf(stream, "]");
}

void
hh_args_print_error(const hh_args_t* args, FILE* stream) {
    HH_ASSERT_INVARIANT(args != NULL);
    HH_ASSERT_INVARIANT(stream != NULL);
    const struct HH__args_error* err = &args->data->error;
    const hh_args_t* origin = args->data->deepest_parsed;
    HH_ASSERT(origin != NULL);
    // print descriptive error message
    switch(err->type) {
    case HH__ARGS_ERR_NONE: return;
    case HH__ARGS_ERR_COMMAND_MISSING:
        fprintf(stream, "Missing required ");
        HH__args_print_error_helper(origin, stream);
        break;
    case HH__ARGS_ERR_COMMAND_DOESNT_EXIST:
        if(err->entry != NULL) 
            fprintf(stream, "Provided argument before required");
        else fprintf(stream, "Invalid");
        fputc(' ', stream);
        HH__args_print_error_helper(origin, stream);
        break;
    case HH__ARGS_ERR_FLAG_MISSING_VALUE:
        HH_ASSERT(err->entry != NULL);
        fprintf(stream, "Flag '" HH__FLAG_FMT "' is missing a required value", 
            HH__FLAG_FMT_ARGS(err->entry->flag, &err->entry->type));
        break;
    case HH__ARGS_ERR_FLAG_INVALID_VALUE:
        HH_ASSERT(err->entry != NULL);
        fprintf(stream, "Flag '" HH__FLAG_FMT "' received an invalid value: %s", 
            HH__FLAG_FMT_ARGS(err->entry->flag, &err->entry->type), err->extra);
        break;
    case HH__ARGS_ERR_FLAG_DUPLICATE:
        HH_ASSERT(err->entry != NULL);
        fprintf(stream, "Flag '" HH__FLAG_FMT "' is passed more than once", 
            HH__FLAG_FMT_ARGS(err->entry->flag, &err->entry->type));
        break;
    case HH__ARGS_ERR_REQUIRED_FLAG_MISSING:
        HH_ASSERT(err->entry != NULL);
        fprintf(stream, "Required flag '" HH__FLAG_FMT "' is missing", 
            HH__FLAG_FMT_ARGS(err->entry->flag, &err->entry->type));
        break;
    case HH__ARGS_ERR_FLAG_INCOMPATIBLE_WITH_COMMAND:
        HH_ASSERT(origin->parent != NULL);
        HH_ASSERT(origin->name != NULL);
        fprintf(stream, "Flag '" HH__FLAG_FMT "' not supported by provided '%s' %s", 
            HH__FLAG_FMT_ARGS(err->entry->flag, &err->entry->type),
            origin->name, origin->parent->parent ? "subcommand" : "command");
        break;
    default: HH_UNREACHABLE;
    }
}

#undef HH__FLAG_FMT_ARGS

static size_t
HH__flag_width(const struct HH__args_entry* entry) {
    size_t col = 0;
    if(!entry->flag.required) col += 2;
    col += 2;
    if(entry->flag.flag == '\0') {
        HH_ASSERT(entry->flag.flag_long != NULL);
        col += strlen(entry->flag.flag_long);
    }
    const char* name = HH__flag_value_name(entry->flag, &entry->type);
    if(name != NULL) col += 3 + strlen(name);
    return col;
}

#define HH__USAGE_OUT(...) if(padding > 0) fprintf(stream, __VA_ARGS__)

static size_t
HH__args_print_usage_entry(const struct HH__args_entry* entry, FILE* stream, 
    _Bool* levels, size_t padding) {
    size_t col = 0;
    for(size_t i = 0; i < hh_darrlen(levels); ++i) {
        HH__USAGE_OUT("%s%*s", levels[i] ? "│" : " ", HH_ARGS_USAGE_INDENT - 2, "");
        col += HH_ARGS_USAGE_INDENT - 1;
    }
    HH__USAGE_OUT(HH__FLAG_FMT, HH__FLAG_FMT_ARGS_SHORT(entry->flag, &entry->type));
    col += HH__flag_width(entry);
    HH_ASSERT(padding == 0 || col <= padding);
    HH__USAGE_OUT("%*s", (int) (padding - col - 1), "");
    if(entry->flag.desc != NULL) {
        HH__USAGE_OUT("%s", entry->flag.desc);
        if(entry->flag.flag != '\0' && entry->flag.flag_long != NULL) {
            HH__USAGE_OUT(" (alt: ");
            if(!entry->flag.required) HH__USAGE_OUT("["); 
            HH__USAGE_OUT("--%s", entry->flag.flag_long);
            if(!entry->flag.required) HH__USAGE_OUT("]"); 
            HH__USAGE_OUT(")"); 
        }
    }
    HH__USAGE_OUT("\n");
    return col;
}

static size_t
HH__args_print_usage_inner(const hh_args_t* args, FILE* stream,
    int argc, char* argv[], _Bool** levels, int last, size_t padding) {
    HH_ASSERT(HH_ARGS_USAGE_INDENT > 2);
    size_t col = 0;
    for(size_t i = 0; i + 1 < hh_darrlen(*levels); ++i) {
        HH__USAGE_OUT("%s%*s", (*levels)[i] ? "│" : " ", HH_ARGS_USAGE_INDENT - 2, "");
        col += HH_ARGS_USAGE_INDENT - 1;
    }
    if(args->parent != NULL) {
        HH__USAGE_OUT("%s", last ? "└" : "├");
        for(size_t i = 0; i < HH_ARGS_USAGE_INDENT - 2; ++i) HH__USAGE_OUT("─");
        HH__USAGE_OUT("%s", args->name);
        col += HH_ARGS_USAGE_INDENT + strlen(args->name);
    } else {
        const char* exe = hh_path_name(argv[0]);
        HH__USAGE_OUT("./%s", exe);
        col += 2 + strlen(exe);
    }
    // print description aligned to the second column
    HH_ASSERT(padding == 0 || col <= padding);
    if(args->desc != NULL) {
        HH__USAGE_OUT("%*s%s", (int) (padding - col), "", args->desc);
    } else if(args->parent == NULL) {
        HH__USAGE_OUT("%*sDESCRIPTION", (int) (padding - col - 1), "");
    }
    HH__USAGE_OUT("\n");
    // print flags
    hh_darrput(*levels, hh_darrlen(args->children) != 0);
    const struct HH__args_entry* entry;
    for(size_t i = 0, j; i < hh_darrlen(args->entries); ++i) {
        entry = (const struct HH__args_entry*) args->entries[i];
        j = HH__args_print_usage_entry(entry, stream, *levels, padding);
        col = HH_MAX(col, j);
    }
    (void) hh_darrpop(*levels);
    // print subcommands
    for(size_t i = 0, j = hh_darrlen(args->children), k; i < j; ++i) {
        hh_darrput(*levels, i != j - 1);
        k = HH__args_print_usage_inner(&args->children[i], stream,
            argc, argv, levels, i == (j - 1), padding);
        col = HH_MAX(col, k);
        (void) hh_darrpop(*levels);
    }
    return col;
}

#undef HH__USAGE_OUT

static void
HH__args_print_synopsis_cmds(const hh_args_t *args, FILE *stream, int argc, char *argv[]) {
    HH_ASSERT(args != NULL);
    if(args->parent != NULL) {
        HH__args_print_synopsis_cmds(args->parent, stream, argc, argv);
        fprintf(stream, "%s ", args->name);
    } else {
        fprintf(stream, "./%s ", hh_path_name(argv[0]));
    }
}

static void
HH__args_print_synopsis_flags(const hh_args_t *args, FILE *stream) {
    HH_ASSERT(args != NULL);
    if(args->parent != NULL) {
        HH__args_print_synopsis_flags(args->parent, stream);
    }
    const struct HH__args_entry* entry;
    for(size_t i = 0; i < hh_darrlen(args->entries); ++i) {
        entry = (const struct HH__args_entry*) args->entries[i];
        fprintf(stream, HH__FLAG_FMT " ", HH__FLAG_FMT_ARGS_SHORT(entry->flag, &entry->type));
    }
}

#undef HH__FLAG_FMT
#undef HH__FLAG_FMT_ARGS_SHORT

static void
HH__args_print_synopsis(const hh_args_t *args, FILE *stream, int argc, char *argv[]) {
    while(hh_darrlen(args->children) == 1) args = &args->children[0];
    HH__args_print_synopsis_cmds(args, stream, argc, argv);
    HH_ASSERT(hh_darrlen(args->children) != 1);
    _Bool cont = 0;
    if(hh_darrlen(args->children) > 1) {
        _Bool sub_one = 0;
        _Bool sub_all = 1;
        _Bool shallow = 1;
        _Bool temp;
        if(args->parent == NULL) {
            fprintf(stream, "<command> ");
            for(size_t i = 0; i < hh_darrlen(args->children); ++i) {
                temp = (hh_darrlen(args->children[i].children) != 0);
                sub_one &= temp;
                sub_all |= temp;
                shallow &= !temp;
            }
            if(shallow) {
                // lazy
            } else if(sub_one) {
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
    HH__args_print_synopsis_flags(args, stream);
    if(cont) fprintf(stream, "...");
    fputc('\n', stream);
}

void
hh_args_print_usage(const hh_args_t* args, FILE* stream, int argc, char* argv[]) {
    HH_ASSERT_INVARIANT(args != NULL);
    HH_ASSERT_INVARIANT(argc > 0);
    HH_ASSERT_INVARIANT(argv[argc] == NULL);
    HH_ASSERT_INVARIANT(args->parent == NULL);
    fprintf(stream, "SYNOPSIS\n");
    HH__args_print_synopsis(args->data->deepest_parsed, stream, argc, argv);
    fputc('\n', stream);
    _Bool* levels = NULL;
    size_t padding = HH__args_print_usage_inner(args, stream, argc, argv, 
        &levels, 1, 0);
    hh_darrclear(levels);
    HH__args_print_usage_inner(args, stream, argc, argv, 
        &levels, 1, padding + HH_ARGS_USAGE_INDENT * 2);
    hh_darrfree(levels);
}

static size_t
HH__ini_skip(hh_span_t s) {
    size_t len = hh_span_len(s);
    size_t incr = 0;
    if(len >= 2 && s.ptr[0] == '\\') {
        if(s.ptr[1] == '\n') incr = 2;
        else if(len >= 3 && s.ptr[1] == '\r' && s.ptr[2] == '\n') incr = 3;
    }
    while(incr > 0 && s.ptr + incr < s.end && isspace(s.ptr[incr])) ++incr;
    return incr;
}

static size_t
HH__ini_trim(hh_span_t s) {
    size_t len = hh_span_len(s);
    if(len >= 2 && s.end[-1] == '\n') {
        if(s.end[-2] == '\\') return 2;
        if(len >= 3 && s.end[-2] == '\r' && s.end[-3] == '\\') return 3;
    }
    return 0;
}

static size_t
HH__ini_size(hh_span_t val) {
    if(hh_span_len(val) == 0) return 0;
    size_t size = 0;
    size_t incr;
    while(val.ptr < val.end) {
        incr = HH__ini_skip(val);
        if(incr > 0) {
            val.ptr += incr;
            continue;
        }
        size++;
        val.ptr++;
    }
    return size;
}

static char*
HH__ini_copy(hh_span_t val, char* buf) {
    hh_darrclear(buf);
    if(hh_span_len(val) == 0) return 0;
    size_t incr;
    while(val.ptr < val.end) {
        incr = HH__ini_skip(val);
        if(incr > 0) {
            val.ptr += incr;
            continue;
        }
        hh_darrput(buf, val.ptr[0]);
        val.ptr++;
    }
    return buf;
}

_Bool
hh_ini_parse(hh_ini_t* ini, hh_span_t* lines, hh_span_t* err) {
    // initialize the top-level node
    hh_ini_t* curr = ini;
    memset(curr, 0, sizeof(hh_ini_t));
    // iterate lines
    char* buf = NULL;
    for(hh_span_t line, line_ext; (line = hh_span_next(lines, .eol = 1, .trim = 1)).ptr;) {
        if(hh_span_len(line) == 0 || line.ptr[0] == ';') 
            continue;
        if(line.end[-1] == '\\') {
            if(hh_span_len(line_ext) == 0) line_ext = line;
            continue;
        }
        if(hh_span_len(line_ext) != 0) {
            line.ptr = line_ext.ptr;
            line_ext.ptr = NULL;
            line_ext.end = NULL;
        }
        *err = line;
        if(line.ptr[0] != '[') {
            hh_span_t raw_key = hh_span_next(&line, .trim = 1, .delim = "=");
            hh_span_t raw_val = hh_span_next(&line, .trim = 1, .delim = ";");
            size_t size_key = HH__ini_size(raw_key);
            size_t size_val = HH__ini_size(raw_val);
            buf = HH__ini_copy(raw_key, buf);
            const char* val = hh_dict_insert(&curr->props,
                buf, size_key, NULL, size_val + 1);
            if(val == NULL) goto failure;
            buf = HH__ini_copy(raw_val, buf);
            hh_darrput(buf, '\0');
            strcpy((char*) val, buf);
            continue;
        }
        curr = ini;
        // advance past '['
        line.ptr++;
        // TODO: potential span_next bug
        // `parser` includes the delim ] when delim_as_set is not supplied
        // delimiters should not be included in the span
        hh_span_t name = hh_span_next(&line, .delim = "]", .delim_as_set = 1);
        // handling line continuations
        for(size_t incr = SIZE_MAX; incr != 0 && hh_span_len(name) > 0; name.end -= incr) {
            incr = 0;
            incr = HH__ini_trim(name);
            if(incr) continue;
            incr = isspace(name.end[-1]) != 0;
            if(incr) continue;
            break;
        }
        hh_span_t name_part;
        while((name_part = hh_span_next(&name, .trim = 1, .delim = ".")).ptr != NULL) {
            buf = HH__ini_copy(name_part, buf);
            // size_t incr = 0;
            // while(incr < hh_darrlen(buf) && isspace(buf[incr])) ++incr;
            hh_ini_t* temp = (void*) hh_dict_get_val(&curr->sections, 
                buf, hh_darrlen(buf));
            // if(incr != 0 && temp != NULL) goto failure;
            if(temp != NULL) {
                curr = (hh_ini_t*) temp;
                continue;
            }
            curr = (hh_ini_t*) hh_dict_insert(&curr->sections, 
                buf, hh_darrlen(buf), NULL, sizeof(hh_ini_t));
            if(curr == NULL) goto failure;
        }
    }
    hh_darrfree(buf);
    memset(err, 0, sizeof(hh_span_t));
    return 1;
failure:
    hh_darrfree(buf);
    return 0;
}

void
hh_ini_free(hh_ini_t* ini) {
    if(ini == NULL) return;
    hh_dict_it(&ini->sections, it) hh_ini_free((hh_ini_t*) it.val);
    hh_dict_free(&ini->sections);
    hh_dict_free(&ini->props);
}

static const hh_ini_t*
HH__ini_query_section(const hh_ini_t* ini, hh_span_t* section) {
    if(hh_span_len(*section) == 0) return NULL;
    hh_span_t name = hh_span_next(section, .delim = ".");
    // printf("query section: " span_fmt " [%zu]\n", span_fmt_args(name), hh_span_len(name));
    return hh_dict_get_val(&ini->sections, name.ptr, hh_span_len(name));
}

const hh_ini_t*
hh_ini_query_section(const hh_ini_t* ini, const char* section) {
    hh_span_t parser = hh_span((char*) section);
    return HH__ini_query_section(ini, &parser);
}

static char*
HH__ini_query(const hh_ini_t* ini, hh_span_t section, const char* key) {
    if(ini == NULL) goto failure;
    const hh_ini_t* ini_sub = HH__ini_query_section(ini, &section);
    // printf("%p\n", ini_sub);
    if(ini_sub == NULL) {
        const char* val = hh_dict_get_val(&ini->props, key, strlen(key));
        if(val != NULL) return (char*) val;
    } else return HH__ini_query(ini_sub, section, key);
failure:
    return NULL;
}

const char*
hh_ini_query(const hh_ini_t* ini, const char* section, const char* key) {
    return HH__ini_query(ini, hh_span((char*) section), key);
}

static _Bool
HH__ini_vscanf(const hh_ini_t* ini, const char* section, 
    const char* key, const char* fmt, int n, va_list arg) {
    const char* val = hh_ini_query(ini, section, key);
    if(val == NULL) return 0;
    size_t len = strlen(val);
    int rc = vsscanf(val, fmt, arg);
    return rc == n && (size_t) ini->n == len;
}

_Bool
HH__ini_scanf(const hh_ini_t* ini, const char* section, 
    const char* key, const char* fmt, int n, ...) {
    va_list arg;
    va_start(arg, n);
    _Bool ret = HH__ini_vscanf(ini, section, key, fmt, n, arg);
    va_end(arg);
    return ret;
}

void
HH__ini_dump(const hh_ini_t* ini, FILE* stream, int level) {
    hh_dict_it(&ini->props, it) {
        fprintf(stream, "%*s\"" hh_span_fmt "\": \"" hh_span_fmt "\"\n", level * HH_INI_INDENT, "",
            (int) it.size_key, (char*) it.key,
            (int) it.size_val, (char*) it.val);
    }
    hh_dict_it(&ini->sections, it) {
        fprintf(stream, "%*s\"" hh_span_fmt "\"\n", level * HH_INI_INDENT, "", 
            (int) it.size_key, (char*) it.key);
        HH__ini_dump(it.val, stream, level + 1);
    }
}

void
hh_ini_dump(const hh_ini_t* ini, FILE* stream) {
    HH__ini_dump(ini, stream, 0);
}

char* 
hh_read_entire_file(const char* path) {
    FILE* f = fopen(path, "rb");
    char* buf = NULL;
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
    buf = calloc(size + 1, sizeof(char*));
    if(buf == NULL) {
        HH_ERR("Failed to allocate buffer for file contents [%s].", path);
        goto failure;
    }
    size_t read_size = fread(buf, 1, size, f);
    if(read_size != size) {
        HH_ERR("Failed to read entire file into buffer [%s].", path);
        goto failure;
    }
    fclose(f);
    return buf;
failure:
    fclose(f);
    free(buf);
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

// NOTE: Great information by Raymond Chen found here:
// https://devblogs.microsoft.com/oldnewthing/20260101-00/?p=111955
void
hh_memswap(char* fst, char* fst_end, char* snd, const char* snd_end) {
    HH_ASSERT_INVARIANT(fst != NULL);
    HH_ASSERT_INVARIANT(fst_end != NULL);
    HH_ASSERT_INVARIANT(snd != NULL);
    HH_ASSERT_INVARIANT(snd_end != NULL);
    HH_ASSERT_INVARIANT(fst <= fst_end);
    HH_ASSERT_INVARIANT(fst_end <= snd);
    HH_ASSERT_INVARIANT(snd <= snd_end);
    hh_memflip(fst, fst_end);
    hh_memflip(fst_end, snd);
    hh_memflip(snd, snd_end);
    hh_memflip(fst, snd_end);
}

void 
hh_memflip(char* ptr, const char* end) {
    HH_ASSERT_INVARIANT(ptr != NULL);
    HH_ASSERT_INVARIANT(end != NULL);
    HH_ASSERT_INVARIANT(ptr <= end);
    hh_memflipn(ptr, (size_t) (end - ptr));
}

void 
hh_memflipn(char* ptr, size_t n) {
    HH_ASSERT_INVARIANT(ptr != NULL);
    if(n == 0) return;
    char temp;
    for(size_t i = 0; i < n / 2; ++i) {
        temp = ptr[i];
        ptr[i] = ptr[n - 1 - i];
        ptr[n - 1 - i] = temp;
    }
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
#define ASSERT HH_ASSERT
#define ASSERT_INVARIANT HH_ASSERT_INVARIANT
#define UNREACHABLE HH_UNREACHABLE
#define malloc_checked hh_malloc_checked
#define calloc_checked hh_calloc_checked
#define fp_wrap_t hh_fp_wrap_t
#define fp_wrap hh_fp_wrap
#define fp_unwrap hh_fp_unwrap
#define ARGS_LENGTH HH_ARGS_LENGTH
#define darrclear hh_darrclear
#define darrfree hh_darrfree
#define darrlast hh_darrlast
#define darrput hh_darrput
#define darrpop hh_darrpop
#define darradd hh_darradd
#define darrlen hh_darrlen
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
#define span_opt hh_span_opt
#define span_len hh_span_len
#define span_fmt hh_span_fmt
#define span_fmt_args hh_span_fmt_args
#define span hh_span
#define span_next hh_span_next
#define span_next_lf hh_span_next_lf
#define span_next_ld hh_span_next_ld
#define span_next_zu hh_span_next_zu
#define hmap_hash_f hh_hmap_hash_f
#define hmap_comp_f hh_hmap_comp_f
#define hmap_free_f hh_hmap_free_f
#define hmap_t hh_hmap_t
#define hmap_entry_t hh_hmap_entry_t
#define hmap_insert hh_hmap_insert
#define hmap_get hh_hmap_get
#define hmap_get_val hh_hmap_get_val
#define hmap_remove hh_hmap_remove
#define hmap_it hh_hmap_it
#define hmap_free hh_hmap_free
#define dict_hash_f hh_dict_hash_f
#define dict_comp_f hh_dict_comp_f
#define dict_free_f hh_dict_free_f
#define dict_t hh_dict_t
#define dict_entry_t hh_dict_entry_t
#define dict_insert hh_dict_insert
#define dict_insert_with_cstr_key hh_dict_insert_with_cstr_key
#define dict_insert_entry hh_dict_insert_entry
#define dict_get hh_dict_get
#define dict_get_with_cstr_key hh_dict_get_with_cstr_key
#define dict_get_val hh_dict_get_val
#define dict_remove hh_dict_remove
#define dict_it hh_dict_it
#define dict_free hh_dict_free
#define args_t hh_args_t
#define flag_type hh_flag_type
#define flag_opt hh_flag_opt
#define args_add_flag hh_args_add_flag
#define args_add_command hh_args_add_command
#define args_parse hh_args_parse
#define args_parsed_cmd hh_args_parsed_cmd
#define args_free hh_args_free
#define args_print_error hh_args_print_error
#define args_print_usage hh_args_print_usage
#define ini_t hh_ini_t
#define ini_parse hh_ini_parse
#define ini_free hh_ini_free
#define ini_query_section hh_ini_query_section
#define ini_query hh_ini_query
#define ini_scanf hh_ini_scanf
#define read_entire_file hh_read_entire_file
#define skip_whitespace hh_skip_whitespace
#define has_prefix hh_has_prefix
#define has_suffix hh_has_suffix
#define memswap hh_memswap
#define memflip hh_memflip
#define memflipn hh_memflipn
//
#endif // HH_STRIP_PREFIXES
//
#endif // not HH__STRIP_PREFIXES
