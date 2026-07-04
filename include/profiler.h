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
hh_profiler_t
hh_profiler_start(const char* name, hh_profiler_t* parent);
// child profilers report their results to the parent, 
// the root profile print their statistics
void
hh_profiler_end(hh_profiler_t* profiler);
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
    _Bool root;
    union {
        struct {
            struct { const char* key;  hh_bench_t val; }* inner;
            char* keys;
        } stats;
        hh_profiler_t* parent;
    } inner;
};
// SECTION(HEADER_PRIVATE, END)

#ifdef HH_IMPLEMENTATION
// SECTION(IMPLEMENTATION)
void
hh_bench_update(hh_bench_t* bench, double entry) {
    bench->count++;
    bench->mean += (entry - bench->mean) / (double) bench->count; 
}

hh_profiler_t
hh_profiler_start(const char* name, hh_profiler_t* parent) {
    HH_ASSERT_INVARIANT(name != NULL);
    HH_ASSERT_INVARIANT(strlen(name) > 0);
    hh_profiler_t profiler = { 
        .name = name, 
        .timer = hh_timer_start(), 
        .root = (parent == NULL) 
    };
    if(profiler.root) {
        hh_hmapconfig(profiler.inner.stats.inner, .key_f = {
                .hash = hh_hash_cstr,
                .comp = hh_comp_cstr
            });
    } else profiler.inner.parent = parent;
    return profiler;
}

static inline void
HH__profiler_full_name(hh_profiler_t* root, const hh_profiler_t* profiler) {
    HH_ASSERT_INVARIANT(root != NULL);
    HH_ASSERT_INVARIANT(profiler != NULL);
    if(!profiler->root) {
        HH__profiler_full_name(root, profiler->inner.parent);
        hh_darrputstr(root->inner.stats.keys, "/");
    }
    hh_darrputstr(root->inner.stats.keys, profiler->name);
}

void
hh_profiler_end(hh_profiler_t* profiler) {
    HH_ASSERT_INVARIANT(profiler != NULL);
    HH_ASSERT_INVARIANT(profiler->name != NULL);
    double elapsed = hh_timer_duration(profiler->timer);
    if(profiler->root) {
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
        hh_profiler_t* root = profiler->inner.parent;
        while(!root->root) root = root->inner.parent;
        // construct the current profiler's full name
        size_t offset = hh_darrlen(root->inner.stats.keys);
        // NOTE: this is done under the assumption that hh_darrputstr 
        // only strips one instance of '\0' at the end of the string
        hh_darrput(root->inner.stats.keys, '\0');
        HH__profiler_full_name(root, profiler);
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
// SECTION(PREFIX, END)
#endif // HH_APPLY_PREFIXES
#endif // not HH__APPLY_PREFIXES
