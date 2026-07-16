#ifndef HH__
#define HH__

// SECTION(HEADER)
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
#ifndef __USE_MINGW_ANSI_STDIO
#define __USE_MINGW_ANSI_STDIO
#endif // not __USE_MINGW_ANSI_STDIO
#endif // __MINGW32__
#endif // _WIN32

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#ifdef _WIN32
#include <winnt.h>
#else // _WIN32
#include <sys/time.h>
#endif // not _WIN32

// to enable logging, define HH_LOG to be one of the following
#define HH_LOG_ERR (1 << 0) // log only errors
#define HH_LOG_MSG (1 << 1) // log info and errors
#define HH_LOG_DBG (1 << 2) // log everything

// change the streams for different log levels through bitflags:
// hh_log_stream_set(HH_LOG_ERR | HH_LOG_DBG, fopen("log.txt", "a"))
// passing NULL resets the standard log stream for that level
void
hh_log_stream_set(int level, FILE* stream);
// unlike the setter, this does not accept multiple log levels
FILE*
hh_log_stream_get(int level);

// all logging functions have the same behavior as printf,
// HH_ERR logs to stderr instead of stdout
#ifdef HH_LOG
#if HH_LOG >= HH_LOG_DBG
#define HH_DBG(...) do { \
    fprintf(hh_log_stream_get(HH_LOG_DBG), "DEBUG [%s:%d]: ", __FILE__, __LINE__); \
    fprintf(hh_log_stream_get(HH_LOG_DBG), __VA_ARGS__); \
    fputc('\n', hh_log_stream_get(HH_LOG_DBG)); \
} while(0)
#else // HH_LOG >= HH_LOG_DBG
#define HH_DBG(...)
#endif // HH_LOG < HH_LOG_DBG
#if HH_LOG >= HH_LOG_MSG
#define HH_MSG(...) do { \
    fprintf(hh_log_stream_get(HH_LOG_MSG), "INFO [%s:%d]: ", __FILE__, __LINE__); \
    fprintf(hh_log_stream_get(HH_LOG_MSG), __VA_ARGS__); \
    fputc('\n', hh_log_stream_get(HH_LOG_MSG)); \
} while(0)
#else // HH_LOG >= HH_LOG_MSG
#define HH_MSG(...)
#endif // HH_LOG < HH_LOG_MSG
#if HH_LOG >= HH_LOG_ERR
#define HH_ERR(...) do { \
    fprintf(hh_log_stream_get(HH_LOG_ERR), "ERROR [%s:%d]: ", __FILE__, __LINE__); \
    fprintf(hh_log_stream_get(HH_LOG_ERR), __VA_ARGS__); \
    fputc('\n', hh_log_stream_get(HH_LOG_ERR)); \
} while(0)
#else // HH_LOG >= HH_LOG_ERR
#define HH_ERR(...)
#endif // HH_LOG < HH_LOG_ERR
#else // HH_LOG
#define HH_DBG(...)
#define HH_MSG(...)
#define HH_ERR(...)
#endif // not HH_LOG

// the block logging functions allow building a log message incrementally,
// useful for printing arrays, etc.
// within the block, HH_LOG_APPEND extends the log
// the log statement produced is automatically newline-terminated 
#ifdef HH_LOG
#if HH_LOG >= HH_LOG_DBG
#define HH_DBG_BLOCK HH_LOG_BLOCK(hh_log_stream_get(HH_LOG_DBG), "DEBUG")
#else
#define HH_DBG_BLOCK if(0)
#endif // HH_DBG
#if HH_LOG >= HH_LOG_MSG
#define HH_MSG_BLOCK HH_LOG_BLOCK(hh_log_stream_get(HH_LOG_MSG), "INFO")
#else
#define HH_MSG_BLOCK if(0)
#endif // HH_MSG
#if HH_LOG >= HH_LOG_ERR
#define HH_ERR_BLOCK HH_LOG_BLOCK(hh_log_stream_get(HH_LOG_ERR), "ERROR")
#else
#define HH_ERR_BLOCK if(0)
#endif // HH_ERR
#define HH_LOG_APPEND(...) fprintf((FILE*) HH_LOG_BLOCK_stream, __VA_ARGS__)
#else
#define HH_DBG_BLOCK if(0)
#define HH_MSG_BLOCK if(0)
#define HH_ERR_BLOCK if(0)
#define HH_LOG_APPEND(...) {}
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
#define hh_realloc_checked(ptr, size) HH__realloc_checked((void**) &(ptr), (size), __FILE__, __LINE__)

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
// hh_darrputstrn  same as above, just copies the first n-characters of the string
// hh_darrpop      removes the last element and returns it by value
// hh_darradd      adds n zero-initialized elements to the array, returns the index to the 1st new element
// hh_darrlen      returns array length
// hh_darrcap      returns array capacity
// hh_darrswap     swaps the elements at 2 indices
// hh_darrswapdel  deletes the ith element by swapping it with the last element, then popping

#define hh_darrclear(arr)           (((arr) == NULL) ? 0 : (hh_darrheader(arr)->len = 0))
#define hh_darrfree(arr)            ((void) (((arr) == NULL) ? (void) 0 : free(hh_darrheader(arr))), (arr) = NULL)
#define hh_darrlast(arr)            ((arr)[hh_darrheader(arr)->len - 1])
#define hh_darrput(arr, val)        ((void) hh_darrgrow(arr, 1), (arr)[(hh_darrheader(arr)->len)++] = (val))
#define hh_darrputstr(arr, str)     (HH__darrputstr((void**) &(arr), (str)))
#define hh_darrputstrn(arr, str, n) (HH__darrputstrn((void**) &(arr), (str), (n)))
#define hh_darrpop(arr)             ((arr)[--(hh_darrheader(arr)->len)])
#define hh_darradd(arr, n)          (HH__darraddn((void**) &(arr), (n), sizeof *(arr)))
#define hh_darrlen(arr)             (((arr) == NULL) ? 0 : hh_darrheader(arr)->len)
#define hh_darrcap(arr)             (((arr) == NULL) ? 0 : hh_darrheader(arr)->cap)
#define hh_darrswap(arr, i, j)      (HH__darrswap((arr), (i), (j)))
#define hh_darrswapdel(arr, i)      (HH__darrswap((arr), (i), hh_darrlen(arr) - 1), hh_darrpop(arr))

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

// high-precision cross-platform timer
typedef struct HH__timer_t hh_timer_t;

// begin timer
hh_timer_t
hh_timer_start(void);
// get time since hh_timer_start (in milliseconds)
double
hh_timer_duration(hh_timer_t from);

// function types for hashing and comparing hmap keys
// in both cases, the pointers... point to the key's bytes
typedef size_t (*hh_hash_f)(const void* ptr, size_t sz);
typedef int    (*hh_comp_f)(const void* fst, const void* snd, size_t sz);
typedef void   (*hh_copy_f)(void* dst, const void* key, size_t sz);

// default hash implementation
size_t
hh_hash_djb2(const void* ptr, size_t sz);

// implementation for cstr keys
// EXAMPLE (non-owning):
// struct { const char* key; int val; }* map = NULL;
// hh_hmapconfig(map, .key_f.hash = hh_hash_cstr, .key_f.comp = hh_comp_cstr);
// const char* temp = "hello";
// hh_hmapinsert(map, &temp, 42);
// EXAMPLE (owning):
// struct { const char key[32]; int val; }* map = NULL;
// hh_hmapconfig(map, .key_f.hash = hh_hash_cstr, .key_f.comp = hh_comp_cstr_owned, .key_f.copy = hh_copy_cstr);
// const char* temp = "hello";
// hh_hmapinsert(map, &temp, 42);
size_t
hh_hash_cstr(const void* ptr, size_t sz);
int
hh_comp_cstr(const void* fst, const void* snd, size_t sz);
int
hh_comp_cstr_owned(const void* fst, const void* snd, size_t sz);
void
hh_copy_cstr(void* dst, const void* ptr, size_t sz);

// configuration options for hmap
// applied through hh_hmapconfig
// see example for cstr helper functions above
typedef struct {
    struct {
        hh_hash_f hash;
        hh_comp_f comp;
        hh_copy_f copy;
        void (*free)(void* ptr);
    } key_f;
    struct {
        void (*free)(void* ptr);
    } val_f;
    size_t reserve;
    size_t bucket_count;
} hh_hmap_opt;

// hh_hmaplen     returns number of entries in the map
// hh_hmapconfig  set custom hh_hmap_opt fields
// hh_hmapinsert  insert an entry, returns a pointer to the replaced entry if the key was already present
// hh_hmapget     returns the index of the entry with the given key
// hh_hmapfree    frees the hmap and sets it to NULL
// hh_hmapremove  removes an entry and returns a pointer to it

#define hh_hmaplen(map)                 (((map) == NULL) ? 0 : hh_hmapheader(map)->len)
#define hh_hmapconfig(map, ...)         ((void) HH__hmapconfig((void**) &(map), hh_hmapprop(map), (hh_hmap_opt) { __VA_ARGS__ }))
#define hh_hmapinsert(map, key_, val_)  (HH__hmapinsert((void**) &(map), hh_hmapprop(map), (key_)) ? \
    ((map)[hh_hmapheader(map)->last].val = val_, &(map)[hh_hmaplen(map)]) : \
    ((map)[hh_hmapheader(map)->last].val = val_, NULL))

// NOTE: if .key_f.free and/or .val_f.free is configured
// the corresponding entry fields will be freed under the following conditions
// - an entry is replaced by hh_hmapinsert
// - an entry is removed by hh_hmapremove
// - an entry was left in the map when hh_hmapfree was called

size_t
hh_hmapget(const void* map, const void* key);
void
hh_hmapfree(const void* map);
void*
hh_hmapremove(const void* map, const void* key);

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
#ifdef HH_LOG
#define HH_LOG_BLOCK(stream, name) for(uintptr_t \
    HH_LOG_BLOCK_stream  = (uintptr_t) (stream), \
    HH_LOG_BLOCK_toggle  = (uintptr_t) (HH_LOG_APPEND("%s [%s:%d]: ", (name), __FILE__, __LINE__) == 0); \
    HH_LOG_BLOCK_toggle != (uintptr_t) '\n' && HH_LOG_BLOCK_toggle != (uintptr_t) EOF; \
    HH_LOG_BLOCK_toggle  = (uintptr_t) fputc('\n', (FILE*) HH_LOG_BLOCK_stream)) // TODO: Consider if this closing newline should be removed
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
void*
HH__realloc_checked(void** ptr, size_t size, const  char* file, int line);

// initial capacity of dynamic array
#ifndef HH_DARR_INITIAL_CAPACITY
#define HH_DARR_INITIAL_CAPACITY 16
#endif // not HH_DARR_INITIAL_CAPACITY

// internal array components
typedef struct { 
    size_t len, cap, elem_size; 
} hh_darrheader_t;

// helper macros for dynamic array implementation
#define hh_darrheader(arr)  (((hh_darrheader_t*) (arr)) - 1)
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
char*
HH__darrputstrn(void** arr_ptr, const char* str, size_t n);

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

struct HH__timer_t {
#ifdef _WIN32
    LARGE_INTEGER start, freq;
#else // _WIN32
    struct timeval start;
#endif // not _WIN32
};

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

// the default number of buckets to be used by hh_hmap
#ifndef HH_BUCKET_COUNT
#define HH_BUCKET_COUNT 16
#endif // not HH_BUCKET_COUNT

// descriptor for hmap entry, computed via hh_hmapprop
typedef struct {
    size_t sz_key, off_key;
    size_t sz_val, off_val;
    size_t sz_entry;
} hh_hmapprop_t;
// compute descriptor for hmap entry
#define hh_hmapprop(map) ((hh_hmapprop_t) { \
    .sz_key = sizeof((map)->key), .off_key = (size_t) ((char*)&(map)->key - (char*)(map)), \
    .sz_val = sizeof((map)->val), .off_val = (size_t) ((char*)&(map)->val - (char*)(map)), \
    .sz_entry = sizeof(*(map)) })

// internal hmap components
typedef struct {
    hh_hmapprop_t prop;
    hh_hmap_opt opt;
    size_t len, cap, last;
    size_t** buckets;
} hh_hmapheader_t;
// macro for retrieving hmap header
#define hh_hmapheader(map) (((hh_hmapheader_t*) (map)) - 1)

// implementations of hmap macros
hh_hmapheader_t*
HH__hmapconfig(void** map_ptr, hh_hmapprop_t prop, hh_hmap_opt opt);
_Bool
HH__hmapinsert(void** map_ptr, hh_hmapprop_t prop, const void* key);

// NetBSD: getline.c,v 1.2 2014/09/16 17:23:50 christos Exp
ptrdiff_t // NO PREFIX STRIPPING
hh_getdelim(char** buf, size_t* bufsiz, int delimiter, FILE* fp);
ptrdiff_t // NO PREFIX STRIPPING
hh_getline(char** buf, size_t* bufsiz, FILE* fp);
// NetBSD: strnlen.c,v 1.2 2014/01/09 11:25:11 apb Exp
size_t
hh_strnlen(const char *s, size_t maxlen);
// SECTION(HEADER_PRIVATE, END)

#ifdef HH_IMPLEMENTATION
// SECTION(IMPLEMENTATION)
// implementation-exclusive includes
#include <ctype.h>
#include <errno.h>
#include <string.h>
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

static FILE* HH_DBG_STREAM = NULL;
static FILE* HH_MSG_STREAM = NULL;
static FILE* HH_ERR_STREAM = NULL;

void
hh_log_stream_set(int level, FILE* stream) {
    if(HH_LOG_DBG & level) HH_DBG_STREAM = stream;
    if(HH_LOG_MSG & level) HH_MSG_STREAM = stream;
    if(HH_LOG_ERR & level) HH_ERR_STREAM = stream;
    HH_UNREACHABLE;
}

FILE*
hh_log_stream_get(int level) {
    if(HH_LOG_DBG & level) return (HH_DBG_STREAM == NULL) ? stdout : HH_DBG_STREAM;
    if(HH_LOG_MSG & level) return (HH_MSG_STREAM == NULL) ? stdout : HH_MSG_STREAM;
    if(HH_LOG_ERR & level) return (HH_ERR_STREAM == NULL) ? stderr : HH_ERR_STREAM;
    HH_UNREACHABLE;
}

void*
HH__malloc_checked(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if(ptr == NULL) {
        fprintf((HH_ERR_STREAM == NULL) ? stdout : HH_ERR_STREAM, "ERROR [%s:%d]: "
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
        fprintf((HH_ERR_STREAM == NULL) ? stdout : HH_ERR_STREAM, "ERROR [%s:%d]: "
            "Failed to allocate %llu objects [%llu bytes]\n", 
            file, line, (unsigned long long) num, (unsigned long long) (size * num));
        abort();
    }
    return ptr;
}

void*
HH__realloc_checked(void** ptr, size_t size, const char* file, int line) {
    void* tmp = *ptr;
    void* ret = realloc(tmp, size);
    if(ret == NULL) {
        fprintf((HH_ERR_STREAM == NULL) ? stdout : HH_ERR_STREAM, "ERROR [%s:%d]: "
            "Failed to allocate %llu bytes\n", 
            file, line, (unsigned long long) size);
        abort();
    }
    *ptr = ret;
    return ret;
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

char*
HH__darrputstrn(void** arr_ptr, const char* str, size_t n) {
    HH_ASSERT_INVARIANT(arr_ptr != NULL);
    HH_ASSERT_INVARIANT(arr_ptr[0] == NULL || (arr_ptr[0] != NULL && hh_darrheader(arr_ptr[0])->elem_size == 1));
    if(hh_darrlen(*arr_ptr) > 0) {
        HH_ASSERT_INVARIANT(hh_darrlast(((char**) arr_ptr)[0]) == '\0');
        (void) hh_darrpop(((char**) arr_ptr)[0]);
    }
    size_t len = hh_strnlen(str, n);
    size_t off = hh_darradd(((char**) arr_ptr)[0], n);
    memcpy(((char**) arr_ptr)[0] + off, str, len);
    if(len < n) memset(((char**) arr_ptr)[0] + off + len, '\0', n - len);
    hh_darrlast(((char**) arr_ptr)[0]) = '\0';
    return ((char**) arr_ptr)[0];
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
        size_t sz_align = (sz + sizeof(void*) - 1) & ~(sizeof(void*) - 1);
        size_t sz_alloc = HH_MAX(HH_ARENA_DEFAULT_SIZE, sz_align);
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
#define PATH_ROOT_LEN 3
#else
#define PATH_ROOT_LEN 1
#endif
    if(path == NULL || hh_darrlen(path) <= PATH_ROOT_LEN) return NULL;
    for(size_t i = 0; path[i]; i++) if(path[i] == '\\') path[i] = '/';
    size_t len = hh_darrlen(path);
    if(len > (PATH_ROOT_LEN + 1) && path[len - 2] == '/' && path[len - 1] == '\0') {
        path[len - 2] = '\0';
        hh_darrheader(path)->len -= 1;
    }
    return path;
#undef PATH_ROOT_LEN
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

hh_timer_t
hh_timer_start(void) {
    hh_timer_t timer;
#ifdef _WIN32
    QueryPerformanceFrequency(&timer.freq);
    QueryPerformanceCounter(&timer.start);
#else // _WIN32
    gettimeofday(&timer.start, NULL);
#endif // not _WIN32
    return timer;
}

double
hh_timer_duration(hh_timer_t timer) {
#ifdef _WIN32
    LARGE_INTEGER end;
    QueryPerformanceCounter(&end);
    // compute and print the elapsed time in millisec
    return (end.QuadPart - timer.start.QuadPart) * 1000.0 / timer.freq.QuadPart;
#else // _WIN32
    struct timeval end;
    gettimeofday(&end, NULL);
    struct timeval dur;
    dur.tv_usec = end.tv_usec - timer.start.tv_usec;
    dur.tv_sec = end.tv_sec - timer.start.tv_sec;
    return (double) (dur.tv_sec * 1000) + ((double) (dur.tv_usec)) / 1000.0;
#endif // not _WIN32
}

size_t
hh_hash_djb2(const void* ptr, size_t sz) {
    size_t hash = 5381;
    for(size_t i = 0; i < sz; ++i) hash = ((hash << 5) + hash) + (size_t) ((char*) ptr)[i];
    return hash;
}

size_t
hh_hash_cstr(const void* ptr, size_t sz) {
    (void) sz;
    const char* str = *((const char**) ptr);
    return hh_hash_djb2(str, strlen(str));
}

int
hh_comp_cstr(const void* fst, const void* snd, size_t sz) {
    (void) sz;
    return strcmp(*((const char**) fst), *((const char**) snd));
}

int
hh_comp_cstr_owned(const void* fst, const void* snd, size_t sz) {
    (void) sz;
    return strcmp(*((const char**) fst), (const char*) snd);
}

void
hh_copy_cstr(void* dst, const void* ptr, size_t sz) {
    const char* str = *((const char**) ptr);
    size_t len_name = HH_MIN(sz - 1, strlen(str));
    ((char*) dst)[len_name] = '\0';
    memcpy(dst, str, len_name);
}

static inline void*
HH__hmapgrow(void** map_ptr, size_t n) {
    HH_ASSERT_INVARIANT(map_ptr != NULL);
    hh_hmapheader_t* map_hdr = hh_hmapheader(map_ptr[0]);
    if(map_hdr->len + n < map_hdr->cap) return map_hdr;
    while(map_hdr->len + n >= map_hdr->cap) map_hdr->cap *= 2;
    map_hdr = realloc(map_hdr, sizeof(hh_hmapheader_t) + map_hdr->cap * map_hdr->prop.sz_entry);
    HH_ASSERT(map_hdr != NULL, "hmapgrow failed to allocate");
    *map_ptr = (void*) (map_hdr + 1);
    return map_hdr;
}

hh_hmapheader_t*
HH__hmapconfig(void** map_ptr, hh_hmapprop_t prop, hh_hmap_opt opt) {
    size_t cap = (opt.reserve > 0) ? opt.reserve : HH_DARR_INITIAL_CAPACITY;
    size_t size = sizeof(hh_hmapheader_t) + prop.sz_entry * cap;
    hh_hmapheader_t* map_hdr = calloc(1, size);
    HH_ASSERT(map_hdr != NULL, "hmapinsert failed to allocate");
    map_hdr->prop = prop;
    map_hdr->opt = opt;
    if(opt.key_f.hash == NULL) map_hdr->opt.key_f.hash = hh_hash_djb2;
    if(opt.key_f.comp == NULL) map_hdr->opt.key_f.comp = memcmp;
    map_hdr->cap = cap;
    map_hdr->last = SIZE_MAX;
    if(opt.bucket_count == 0) map_hdr->opt.bucket_count = HH_BUCKET_COUNT;
    map_hdr->buckets = calloc(map_hdr->opt.bucket_count, sizeof(size_t*));
    HH_ASSERT(map_hdr != NULL, "hmapinsert failed to allocate");
    map_ptr[0] = (void*) (map_hdr + 1);
    return map_hdr;
}

static inline size_t
HH__hmapbucketindex(const hh_hmapheader_t* map_hdr, const void* ptr) {
    return (map_hdr->opt.key_f.hash)(ptr, map_hdr->prop.sz_key) % map_hdr->opt.bucket_count;
}

static inline size_t*
HH__hmapbucket(const hh_hmapheader_t* map_hdr, const void* ptr) {
    return map_hdr->buckets[HH__hmapbucketindex(map_hdr, ptr)];
}

_Bool
HH__hmapinsert(void** map_ptr, hh_hmapprop_t prop, const void* key) {
    HH_ASSERT_INVARIANT(map_ptr != NULL);
    hh_hmapheader_t* map_hdr = (map_ptr[0] == NULL) ? \
        HH__hmapconfig(map_ptr, prop, (hh_hmap_opt) {0}) : \
        hh_hmapheader(map_ptr[0]);
    map_hdr = HH__hmapgrow(map_ptr, 1);
    map_hdr->last = SIZE_MAX;
    char* entry_start = ((char*) map_ptr[0]) + map_hdr->prop.sz_entry * map_hdr->len;
    // check if element exists in the map
    size_t idx = hh_hmapget(map_ptr[0], key);
    if(idx < map_hdr->len) {
        // if it does, copy that element to the end so we can return it
        // this prevents data loss
        char* entry_old = ((char*) map_ptr[0]) + map_hdr->prop.sz_entry * idx;
        if(map_hdr->opt.key_f.free != NULL) 
            (map_hdr->opt.key_f.free)(*((void**) entry_old + map_hdr->prop.off_key));
        if(map_hdr->opt.val_f.free != NULL) 
            (map_hdr->opt.val_f.free)(*((void**) entry_old + map_hdr->prop.off_val));
        memcpy(entry_start, entry_old, map_hdr->prop.sz_entry);
        entry_start = entry_old;
        // save the index for use in macro
        map_hdr->last = idx;
    }
    // add the new element
    memset(entry_start, 0, map_hdr->prop.sz_entry);
    if(map_hdr->opt.key_f.copy == NULL) {
        memcpy(entry_start + map_hdr->prop.off_key, key, map_hdr->prop.sz_key);
    } else {
        (map_hdr->opt.key_f.copy)(entry_start + map_hdr->prop.off_key, key, map_hdr->prop.sz_key);
    }
    // add the corresponding bucket entry
    if(idx == SIZE_MAX) {
        hh_darrput(map_hdr->buckets[HH__hmapbucketindex(map_hdr, key)], map_hdr->len);
        // increment length and save the index for use in macro
        map_hdr->last = map_hdr->len++;
    }
    return map_hdr->last == idx;
}

size_t
hh_hmapget(const void* map, const void* key) {
    HH_ASSERT_INVARIANT(key != NULL);
    if(map == NULL) return SIZE_MAX;
    hh_hmapheader_t* map_hdr = hh_hmapheader(map);
    size_t* bucket = HH__hmapbucket(map_hdr, key);
    char* other;
    for(size_t i = 0; i < hh_darrlen(bucket); ++i) {
        // calculate byte offset to entry's key
        other = (char*) map + bucket[i] * map_hdr->prop.sz_entry + map_hdr->prop.off_key;
        if((map_hdr->opt.key_f.comp)(key, other, map_hdr->prop.sz_key) == 0) 
            return bucket[i];
    }
    return SIZE_MAX;
}

// TODO: Should shadow with macro to set map to NULL
void
hh_hmapfree(const void* map) {
    if(map == NULL) return;
    hh_hmapheader_t* map_hdr = hh_hmapheader(map);
    if(map_hdr->opt.key_f.free != NULL) {
        char* key;
        for(size_t i = 0; i < map_hdr->len; ++i) {
            key = (char*) map + i * map_hdr->prop.sz_entry + map_hdr->prop.off_key;
            (map_hdr->opt.key_f.free)(*((void**) key));
        }
    }
    if(map_hdr->opt.val_f.free != NULL) {
        char* val;
        for(size_t i = 0; i < map_hdr->len; ++i) {
            val = (char*) map + i * map_hdr->prop.sz_entry + map_hdr->prop.off_val;
            (map_hdr->opt.val_f.free)(*((void**) val));
        }
    }
    for(size_t i = 0; i < map_hdr->opt.bucket_count; ++i) {
        hh_darrfree(map_hdr->buckets[i]);
    }
    free(map_hdr->buckets);
    free(map_hdr);
}

void*
hh_hmapremove(const void* map, const void* key) {
    HH_ASSERT_INVARIANT(key != NULL);
    if(map == NULL) return NULL;
    if(hh_hmaplen(map) == 0) return NULL;
    // make sure key exists in the map
    size_t idx = hh_hmapget(map, key);
    if(idx == SIZE_MAX) return NULL;
    // perform deletion
    hh_hmapheader_t* map_hdr = hh_hmapheader(map);
    (map_hdr->len)--;
    char* fst = (char*) map + idx * map_hdr->prop.sz_entry;
    char* snd = (char*) map + map_hdr->len * map_hdr->prop.sz_entry;
    // remove the bucket entry for the deleted entry
    size_t* bucket = HH__hmapbucket(map_hdr, fst + map_hdr->prop.off_key);
    _Bool found = 0;
    for(size_t i = 0; i < hh_darrlen(bucket); ++i) {
        if(bucket[i] == idx) {
            hh_darrswapdel(bucket, i);
            found = 1;
            break;
        }
    }
    HH_ASSERT(found);
    // if there are >1 elements in the map we do swap deletion
    _Bool swap = (idx < map_hdr->len);
    if(swap) {
        char* fst_end = fst + map_hdr->prop.sz_entry;
        char* snd_end = snd + map_hdr->prop.sz_entry;
        hh_memswap(fst, fst_end, snd, snd_end);
        // update the bucket entry for the moved entry
        bucket = HH__hmapbucket(map_hdr, fst + map_hdr->prop.off_key);
        found = 0;
        for(size_t i = 0; i < hh_darrlen(bucket); ++i) {
            if(bucket[i] == map_hdr->len) {
                bucket[i] = idx;
                found = 1;
                break;
            }
        }
        HH_ASSERT(found);
    }
    char* entry_start = (swap ? snd : fst);
    if(map_hdr->opt.key_f.free != NULL) 
        (map_hdr->opt.key_f.free)(*((void**) (entry_start + map_hdr->prop.off_key)));
    if(map_hdr->opt.val_f.free != NULL) 
        (map_hdr->opt.val_f.free)(*((void**) (entry_start + map_hdr->prop.off_val)));
    // return pointer to the removed entry
    return entry_start;
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

/*-
 * Copyright (c) 2009 David Schultz <das@FreeBSD.org>
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

size_t
hh_strnlen(const char *s, size_t maxlen) {
	size_t len;
	for(len = 0; len < maxlen; len++, s++) {
		if(!*s) break;
	}
	return (len);
}
// SECTION(IMPLEMENTATION, END)
#endif // HH_IMPLEMENTATION
#endif // HH__

#ifndef HH__APPLY_PREFIXES
#define HH__APPLY_PREFIXES
#ifndef HH_APPLY_PREFIXES
// SECTION(PREFIX)
#define DBG HH_DBG
#define MSG HH_MSG
#define ERR HH_ERR
#define DBG_BLOCK HH_DBG_BLOCK
#define MSG_BLOCK HH_MSG_BLOCK
#define ERR_BLOCK HH_ERR_BLOCK
#define LOG_APPEND HH_LOG_APPEND
#define MAX HH_MAX
#define MIN HH_MIN
#define ARR_LEN HH_ARR_LEN
#define UNUSED HH_UNUSED
#define FALLTHROUGH HH_FALLTHROUGH
#define CONCATENATE HH_CONCATENATE
#define STRINGIFY HH_STRINGIFY
#define STRINGIFY_BOOL HH_STRINGIFY_BOOL
#define ASSERT HH_ASSERT
#define ASSERT_INVARIANT HH_ASSERT_INVARIANT
#define UNREACHABLE HH_UNREACHABLE
#define malloc_checked hh_malloc_checked
#define calloc_checked hh_calloc_checked
#define realloc_checked hh_realloc_checked
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
#define darrputstrn hh_darrputstrn
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
#define timer_t hh_timer_t
#define timer_start hh_timer_start
#define timer_duration hh_timer_duration
#define hash_f hh_hash_f
#define comp_f hh_comp_f
#define hash_djb2 hh_hash_djb2
#define hash_cstr hh_hash_cstr
#define comp_cstr hh_comp_cstr
#define comp_cstr_owned hh_comp_cstr_owned
#define copy_cstr hh_copy_cstr
#define hmap_opt hh_hmap_opt
#define hmapconfig hh_hmapconfig
#define hmaplen hh_hmaplen
#define hmapinsert hh_hmapinsert
#define hmapget hh_hmapget
#define hmapfree hh_hmapfree
#define hmapremove hh_hmapremove
#define read_entire_file hh_read_entire_file
#define skip_whitespace hh_skip_whitespace
#define has_prefix hh_has_prefix
#define has_suffix hh_has_suffix
#define memswap hh_memswap
#define memflip hh_memflip
#define memflipn hh_memflipn
// SECTION(PREFIX, END)
#endif // HH_APPLY_PREFIXES
#endif // not HH__APPLY_PREFIXES
