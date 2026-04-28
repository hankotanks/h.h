#ifndef HH_HMAP__
#define HH_HMAP__

#include "h.h"

typedef size_t (*hh_hash_f)(const void* ptr, size_t sz);
typedef int    (*hh_comp_f)(const void* fst, const void* snd, size_t sz);

size_t
hh_hash_djb2(const void* ptr, size_t sz);

size_t
hh_hash_cstr(const void* ptr, size_t sz);
int
hh_comp_cstr(const void* fst, const void* snd, size_t sz);

typedef struct {
    struct {
        hh_hash_f hash;
        hh_comp_f comp;
        void (*free)(void* ptr);
    } key_f;
    struct {
        void (*free)(void* ptr);
    } val_f;
    size_t reserve;
    size_t bucket_count;
} hh_hmap_opt;

#define hh_hmaplen(map)                 (((map) == NULL) ? 0 : hh_hmapheader(map)->len)
#define hh_hmapconfig(map, ...)         (HH__hmapconfig((void**) &(map), hh_hmapprop(map), (hh_hmap_opt) { __VA_ARGS__ }))
#define hh_hmapinsert(map, key_, val_)  (HH__hmapinsert((void**) &(map), hh_hmapprop(map), (key_)) ? \
    ((map)[hh_hmapheader(map)->last].val = val_, &(map)[hh_hmaplen(map)]) : \
    ((map)[hh_hmapheader(map)->last].val = val_, NULL))

size_t
hh_hmapget(const void* map, const void* key);
void
hh_hmapfree(const void* map);
void*
hh_hmapremove(const void* map, const void* key);

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

typedef struct {
    size_t sz_key, off_key;
    size_t sz_val, off_val;
    size_t sz_entry;
} hh_hmapprop_t;

#define hh_hmapprop(map) ((hh_hmapprop_t) { \
    .sz_key = sizeof((map)->key), .off_key = (size_t) ((char*)&(map)->key - (char*)(map)), \
    .sz_val = sizeof((map)->val), .off_val = (size_t) ((char*)&(map)->val - (char*)(map)), \
    .sz_entry = sizeof(*(map)) })

typedef struct {
    hh_hmapprop_t prop;
    hh_hmap_opt opt;
    size_t len, cap, last;
    size_t** buckets;
} hh_hmapheader_t;

#define hh_hmapheader(map) (((hh_hmapheader_t*) map) - 1)

hh_hmapheader_t*
HH__hmapconfig(void** map_ptr, hh_hmapprop_t prop, hh_hmap_opt opt);
_Bool
HH__hmapinsert(void** map_ptr, hh_hmapprop_t prop, const void* key);
//
#ifdef HH_IMPLEMENTATION
//
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

static void*
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

static size_t
HH__hmapbucketindex(const hh_hmapheader_t* map_hdr, const void* ptr) {
    return (map_hdr->opt.key_f.hash)(ptr, map_hdr->prop.sz_key) % map_hdr->opt.bucket_count;
}

static size_t*
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
    memcpy(entry_start + map_hdr->prop.off_key, key, map_hdr->prop.sz_key);
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
//
#endif // HH_IMPLEMENTATION
//
#endif // HH_HMAP__

#ifndef HH__STRIP_PREFIXES
//
#define HH__STRIP_PREFIXES
//
#ifdef HH_STRIP_PREFIXES
#define hash_f hh_hash_f
#define comp_f hh_comp_f
#define hash_djb2 hh_hash_djb2
#define hash_cstr hh_hash_cstr
#define comp_cstr hh_comp_cstr
#define hmap_opt hh_hmap_opt
#define hmapconfig hh_hmapconfig
#define hmaplen hh_hmaplen
#define hmapinsert hh_hmapinsert
#define hmapget hh_hmapget
#define hmapfree hh_hmapfree
#define hmapremove hh_hmapremove
//
#endif // HH_STRIP_PREFIXES
//
#endif // not HH__STRIP_PREFIXES
