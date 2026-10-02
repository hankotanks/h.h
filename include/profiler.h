#ifndef HH_PROFILER__
#define HH_PROFILER__

#include "core.h"

// SECTION(HEADER)
// simple struct for calculating an incremental average
typedef struct {
    double mean;
    size_t count;
} hh_bench_t;

// add a data point to the benchmark
void
hh_bench_update(hh_bench_t* bench, double entry);

// struct representing a profiler
typedef struct HH__profiler_t hh_profiler_t;

// create a new profiler, these can be nested
void hh_profiler_start(hh_profiler_t* profiler, const char* name, hh_profiler_t* parent);
// child profilers report their results to the parent, 
// the root profile print their statistics
void
hh_profiler_end(hh_profiler_t* profiler);

// macros to avoid tracking hh_profiler_t variables
// just supply a name for the benchmark and optionally the name of the parent, i.e.
// HH_PROFILE(main);
// ...
//    HH_PROFILE(my_function, main);
//    ...
//    HH_PROFILE_END(my_function);
// ...
// HH_PROFILE_END(main);
#define HH_PROFILE(...) HH__PROFILE(__VA_ARGS__)
#define HH_PROFILE_END(name) hh_profiler_end(&_profiler_##name)
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
struct HH__profiler_t {
    const char* name;
    hh_timer_t timer;
    hh_profiler_t* root;
    union {
        struct {
            struct { const char* key;  hh_bench_t val; }* inner;
            char* keys;
            const hh_profiler_t* latest;
        } stats;
        hh_profiler_t* parent;
    } inner;
};

#define HH__PROFILE1(name) NULL
#define HH__PROFILE2(name, parent) & _profiler_##parent
#define HH__PROFILE(...) \
    hh_profiler_t HH__PROFILE_VARIABLE(__VA_ARGS__); \
    hh_profiler_start(&HH__PROFILE_VARIABLE(__VA_ARGS__), HH_STRINGIFY(HH__PROFILE_NAME(__VA_ARGS__)), HH_CONCATENATE(HH__PROFILE, HH_ARGS_LENGTH(__VA_ARGS__))(__VA_ARGS__))

#define HH__PROFILE_NAME1(name) name
#define HH__PROFILE_NAME2(name, ...) name
#define HH__PROFILE_NAME(...) HH_CONCATENATE(HH__PROFILE_NAME, HH_ARGS_LENGTH(__VA_ARGS__))(__VA_ARGS__)

#define HH__PROFILE_VARIABLE1(name) _profiler_##name
#define HH__PROFILE_VARIABLE2(name, ...) _profiler_##name
#define HH__PROFILE_VARIABLE(...) HH_CONCATENATE(HH__PROFILE_VARIABLE, HH_ARGS_LENGTH(__VA_ARGS__))(__VA_ARGS__)
// SECTION(HEADER_PRIVATE, END)

#ifdef HH_IMPLEMENTATION
// SECTION(IMPLEMENTATION)
void
hh_bench_update(hh_bench_t* bench, double entry) {
    bench->count++;
    bench->mean += (entry - bench->mean) / (double) bench->count; 
}

void 
hh_profiler_start(hh_profiler_t* profiler, const char* name, hh_profiler_t* parent) {
    HH_ASSERT_INVARIANT(name != NULL);
    HH_ASSERT_INVARIANT(strlen(name) > 0);
    profiler->name = name;
    profiler->timer = hh_timer_start(),
    profiler->root = (parent == NULL) ? NULL : ((parent->root == NULL) ? parent : parent->root);
    if(profiler->root == NULL) {
        hh_hmapconfig(profiler->inner.stats.inner, .key_f = {
                .hash = hh_hash_cstr,
                .comp = hh_comp_cstr
            });
        profiler->inner.stats.keys = NULL;
        profiler->inner.stats.latest = profiler;
    } else {
        HH_ASSERT(profiler->root->inner.stats.latest == parent, 
            "Profiler '%s' is improperly nested or an hh_profiler_end call was forgotten", name);
        profiler->root->inner.stats.latest = profiler;
        profiler->inner.parent = parent;
    }
}

static inline void
HH__profiler_full_name(const hh_profiler_t* profiler) {
    HH_ASSERT_INVARIANT(profiler != NULL);
    if(profiler->root != NULL) {
        HH__profiler_full_name(profiler->inner.parent);
        hh_darrputstr(profiler->root->inner.stats.keys, "/");
        hh_darrputstr(profiler->root->inner.stats.keys, profiler->name);
    } else {
        hh_darrputstr(profiler->inner.stats.keys, profiler->name);
    }
}

void
hh_profiler_end(hh_profiler_t* profiler) {
    HH_ASSERT_INVARIANT(profiler != NULL);
    HH_ASSERT_INVARIANT(profiler->name != NULL);
    double elapsed = hh_timer_duration(profiler->timer);
    if(profiler->root == NULL) {
        // dump results
        HH_DBG_BLOCK {
            HH_LOG_APPEND("Results from \"%s\" profiler...\n", profiler->name);
            HH_LOG_APPEND("  %s: %.2lfms [1 sample]\n", profiler->name, elapsed);
            hh_bench_t bench;
            for(size_t i = 0; i < hh_hmaplen(profiler->inner.stats.inner); ++i) {
                bench = profiler->inner.stats.inner[i].val;
                HH_LOG_APPEND("  %s: %.2lfms [%zu sample%s]\n", 
                    profiler->inner.stats.inner[i].key, 
                    bench.mean, bench.count, bench.count == 1 ? "" : "s");
            }
            (void) bench;
        }
        hh_hmapfree(profiler->inner.stats.inner);
        hh_darrfree(profiler->inner.stats.keys);
    } else {
        // find the root profiler
        hh_profiler_t* root = profiler->root;
        // construct the current profiler's full name
        size_t offset = hh_darrlen(root->inner.stats.keys);
        // NOTE: this is done under the assumption that hh_darrputstr 
        // only strips one instance of '\0' at the end of the string
        hh_darrput(root->inner.stats.keys, '\0');
        HH__profiler_full_name(profiler);
        const char* key = root->inner.stats.keys + offset;
        // check if it's the first iteration
        size_t idx = hh_hmapget(root->inner.stats.inner, &key);
        if(idx == SIZE_MAX) {
            // insert the first data point for this profiler
            hh_bench_t bench = { .mean = elapsed, .count = 1 };
            hh_hmapinsert(root->inner.stats.inner, &key, bench);
        } else {
            // the key already exists, so remove the one we constructed
            hh_darrheader(root->inner.stats.keys)->len = offset;
            // compute incremental mean
            hh_bench_update(&root->inner.stats.inner[idx].val, elapsed);
        }
        // maintain `latest`
        root->inner.stats.latest = profiler->inner.parent;
    }
}
// SECTION(IMPLEMENTATION, END)
#endif // HH_IMPLEMENTATION
#endif // HH_PROFILER__

#ifndef HH__APPLY_PREFIXES
#define HH__APPLY_PREFIXES
#ifndef HH_APPLY_PREFIXES
// SECTION(PREFIX)
#define bench_t hh_bench_t
#define bench_update hh_bench_update
#define profiler_t hh_profiler_t
#define profiler_start hh_profiler_start
#define profiler_end hh_profiler_end
#define PROFILE HH_PROFILE
#define PROFILE_END HH_PROFILE_END
// SECTION(PREFIX, END)
#endif // HH_APPLY_PREFIXES
#endif // not HH__APPLY_PREFIXES
