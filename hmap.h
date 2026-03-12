#ifndef HH_HMAP__
#define HH_HMAP__

#include "h.h"

typedef size_t (*hh_hash_f)(const void* ptr, size_t ptr_size);
typedef int    (*hh_comp_f)(const void* fst, size_t fst_size, const void* snd, size_t snd_size);

typedef struct {
    size_t key_size;
    hh_hash_f hash;
    hh_comp_f comp;
    size_t reserve;
    size_t bucket_count;
} hh_hmap_opt;

#define hh_hmapconfig(map, ...)                (HH__hmapconfig((void**) &(map), sizeof(*(map)), (hh_hmap_opt) { __VA_ARGS__ }))
#define hh_hmapinsert(map, key, key_size, val) (HH__hmapinsert((void**) &(map), (key), (key_size), (val), sizeof(*(val))))
#define hh_hmaplen(map)                        (((map) == NULL) ? 0 : hh_hmapheader(map)->len)
#define hh_hmapfree(map)                       (HH__hmapfree((void**) &(map)))

size_t
hh_hmapget(void* map, const void* key, size_t key_size);
void*
hh_hmapremove(void* map, const void* key, size_t key_size);

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

// hmap header
typedef struct {
    size_t len;
    size_t cap;
    size_t key_size;
    size_t val_size;
    hh_hash_f hash;
    hh_comp_f comp;
    size_t bucket_count;
    char** buckets;
    void** bucket_lookup;
} hh_hmapheader_t;

#define hh_hmapheader(arr)  (((hh_hmapheader_t*) arr) - 1)

void
HH__hmapconfig(void** map_ptr, size_t val_size, hh_hmap_opt opt);
void*
HH__hmapinsert(void** map_ptr, void* key, size_t key_size, void* val, size_t val_size);
void
HH__hmapfree(void** map_ptr);

#ifdef HH_IMPLEMENTATION
void
HH__hmapconfig(void** map_ptr, size_t val_size, hh_hmap_opt opt) {
    size_t cap = (opt.reserve > 0) ? opt.reserve : HH_DARR_INITIAL_CAPACITY;
    hh_hmapheader_t* map_hdr = calloc(1, sizeof(hh_hmapheader_t) + val_size * cap);
    HH_ASSERT(map_hdr != NULL, "HH__hmapconfig failed to allocate map");
    map_hdr->cap = cap;
    map_hdr->key_size = opt.key_size;
    map_hdr->val_size = val_size;
    map_hdr->hash = opt.hash;
    map_hdr->comp = opt.comp;
    map_hdr->bucket_count = (opt.bucket_count > 0) ? opt.bucket_count : HH_BUCKET_COUNT;
    map_hdr->buckets = calloc(map_hdr->bucket_count, sizeof(char*));
    HH_ASSERT(map_hdr->buckets != NULL, "HH__hmapconfig failed to allocate map");
    map_hdr->bucket_lookup = NULL;
    HH__darrgrow((void**) &map_hdr->bucket_lookup, 0, sizeof(void*));
    (*map_ptr) = (void*) (map_hdr + 1);
}

void*
HH__hmapgrow(void** map_ptr, size_t n) {
    HH_ASSERT_INVARIANT(map_ptr != NULL);
    hh_hmapheader_t* map_hdr = hh_hmapheader(*map_ptr);
    if(map_hdr->len + n < map_hdr->cap) return map_hdr;
    while(map_hdr->len + n >= map_hdr->cap) map_hdr->cap *= 2;
    map_hdr = realloc(map_hdr, sizeof(hh_hmapheader_t) + map_hdr->cap * map_hdr->val_size);
    HH_ASSERT(map_hdr != NULL, "HH__hmapgrow failed to allocate map");
    *map_ptr = (void*) (map_hdr + 1);
    return map_hdr;
}

static size_t
HH__hmap_bucket_index(const void* map, const void* ptr, size_t ptr_size) {
    hh_hmapheader_t* map_hdr = hh_hmapheader(map);
    return ((map_hdr->hash == NULL) ? 
        HH__hash_djb2(ptr, ptr_size) : 
        (map_hdr->hash)(ptr, ptr_size)) % map_hdr->bucket_count;
}

void*
HH__hmapinsert(void** map_ptr, void* key, size_t key_size, void* val, size_t val_size) {
    HH_ASSERT_INVARIANT(map_ptr != NULL);
    if(map_ptr[0] == NULL) HH__hmapconfig(map_ptr, val_size, (hh_hmap_opt) {0});
    hh_hmapheader_t* map_hdr = hh_hmapheader(map_ptr[0]);
    HH_ASSERT_INVARIANT(key_size > 0 || (key_size == 0 && map_hdr->key_size > 0));
    key_size = (map_hdr->key_size == 0) ? key_size : map_hdr->key_size;
    // NOTE: This serves as 'type-checking' for the map
    // If the original type has been lost, the validity of the map cannot be guaranteed
    HH_ASSERT_INVARIANT(val_size == map_hdr->val_size);
    size_t bucket_index = HH__hmap_bucket_index(map_ptr[0], key, key_size);
    size_t entry_size;
    if(map_hdr->key_size == 0) entry_size = key_size + sizeof(size_t) * 2;
    else entry_size = map_hdr->key_size + sizeof(size_t);
    size_t entry_index = hh_darradd(map_hdr->buckets[bucket_index], entry_size);
    char* entry_start = map_hdr->buckets[bucket_index] + entry_index;
    char* entry_end = entry_start;
    {
        if(map_hdr->key_size == 0) {
            *((size_t*) entry_end) = key_size;
            entry_end += sizeof(size_t);
            memcpy(entry_end, key, key_size);
            entry_end += key_size;
        } else {
            memcpy(entry_end, key, map_hdr->key_size);
            entry_end += map_hdr->key_size;
        }
        *((size_t*) entry_end) = map_hdr->len;
        // NOTE: `entry_end` actually points to beginning of value index in the entry
    }
    // add value to dynamic array
    size_t val_index = map_hdr->len;
    map_hdr = HH__hmapgrow(map_ptr, 1);
    map_hdr->len++;
    char* val_start = (char*) map_ptr[0] + val_index * map_hdr->val_size;
    memcpy(val_start , val, map_hdr->val_size);
    // add entry in lookup table
    hh_darrput(map_hdr->bucket_lookup, entry_end);
    return val_start;
}

static int
HH__hmap_comp(const void* map, const void* fst, size_t fst_size, const void* snd, size_t snd_size) {
    hh_hmapheader_t* map_hdr = hh_hmapheader(map);
    if(map_hdr->comp != NULL) return (map_hdr->comp)(fst, fst_size, snd, snd_size);
    int result = memcmp(fst, snd, HH_MIN(fst_size, snd_size));
    if(result != 0) return result;
    if(fst_size < snd_size) return -1;
    if(fst_size > snd_size) return 1;
    return 0;
}

size_t
hh_hmapget(void* map, const void* key, size_t key_size) {
    HH_ASSERT_INVARIANT(map != NULL);
    HH_ASSERT_INVARIANT(key != NULL);
    hh_hmapheader_t* map_hdr = hh_hmapheader(map);
    HH_ASSERT_INVARIANT(map_hdr->key_size > 0 || (map_hdr->key_size == 0 && key_size > 0));
    key_size = (map_hdr->key_size == 0) ? key_size : map_hdr->key_size;
    size_t bucket_index = HH__hmap_bucket_index(map, key, key_size);
    char* bucket = map_hdr->buckets[bucket_index];
    if(bucket == NULL) return SIZE_MAX;
    char* ptr = bucket;
    do {
        size_t ptr_size = map_hdr->key_size;
        if(map_hdr->key_size == 0) {
            ptr_size = *((size_t*) ptr);
            ptr += sizeof(size_t);
        }
        if(HH__hmap_comp(map, key, key_size, ptr, ptr_size) == 0) {
            return *((size_t*) (ptr + ptr_size));
        }
        ptr += ptr_size + sizeof(size_t);
    } while(ptr < (bucket + hh_darrlen(bucket)));
    return SIZE_MAX;
}

void*
hh_hmapremove(void* map, const void* key, size_t key_size) {
    HH_ASSERT_INVARIANT(map != NULL);
    HH_ASSERT_INVARIANT(key != NULL);
    hh_hmapheader_t* map_hdr = hh_hmapheader(map);
    HH_ASSERT_INVARIANT(map_hdr->key_size > 0 || (map_hdr->key_size == 0 && key_size > 0));
    char* bucket_start = map_hdr->buckets[HH__hmap_bucket_index(map, key, key_size)];
    if(bucket_start == NULL) return NULL;
    char* entry_start = bucket_start;
    size_t entry_key_size = map_hdr->key_size;
    size_t entry_size = map_hdr->key_size + sizeof(size_t);
    size_t entry_index;
    do {
        if(map_hdr->key_size == 0) {
            entry_key_size = *((size_t*) entry_start);
            entry_start += sizeof(size_t);
        }  
        if(HH__hmap_comp(map, key, key_size, entry_start, entry_key_size) == 0) {
            if(map_hdr->key_size == 0) {
                entry_start -= sizeof(size_t);
                entry_size = sizeof(size_t) + entry_key_size + sizeof(size_t);
                entry_index = *((size_t*) (entry_start + sizeof(size_t) + entry_key_size));
            } else {
                entry_index = *((size_t*) (entry_start + entry_key_size));
            }
            goto found;
        }
        entry_start += entry_key_size + sizeof(size_t);
    } while(entry_start < (bucket_start + hh_darrlen(bucket_start)));
    return NULL;
found:
    (void) NULL;
    char* bucket_end = bucket_start + hh_darrlen(bucket_start);
    char* entry_end = entry_start + entry_size;
    assert(entry_end <= bucket_end);
    if(entry_end < bucket_end) {
        hh_memswap(entry_start, entry_end, entry_end, bucket_end);
        // need to correct indices for all entries [entry_start, bucket_end - entry_size]
        size_t ptr_key_size = map_hdr->key_size;
        char* ptr = entry_start;
        while(ptr < bucket_end - entry_size) {
            if(map_hdr->key_size == 0) {
                ptr_key_size = *((size_t*) ptr);
                ptr += sizeof(size_t);
            }  
            ptr += ptr_key_size;
            map_hdr->bucket_lookup[*((size_t*) ptr)] = ptr;
            ptr += sizeof(size_t);
        }
    }
    hh_darrheader(bucket_start)->len -= entry_size;
    // remove the actual value from the dynamic array
    assert(hh_hmaplen(map) > entry_index);
    map_hdr->len--;
    if(entry_index == hh_hmaplen(map)) {
        (void) hh_darrpop(map_hdr->bucket_lookup);
        return (char*) map + entry_index * map_hdr->val_size;
    }
    // swap deletion
    char* src_start = (char*) map + entry_index * map_hdr->val_size;
    char* src_end = src_start + map_hdr->val_size;
    char* dst_start = (char*) map + hh_hmaplen(map) * map_hdr->val_size;
    char* dst_end = dst_start + map_hdr->val_size;
    hh_memswap(src_start, src_end, dst_start, dst_end);
    // remove the actual value and update the index of swapped element
    hh_darrswapdel(map_hdr->bucket_lookup, entry_index);
    // entry_index now points to the previous last element
    *((size_t*) map_hdr->bucket_lookup[entry_index]) = entry_index;
    // return a pointer to the removed value
    return (char*) map + hh_hmaplen(map) * map_hdr->val_size;
}

void
HH__hmapfree(void** map_ptr) {
    HH_ASSERT_INVARIANT(map_ptr != NULL);
    if(map_ptr[0] == NULL) return;
    hh_hmapheader_t* map_hdr = hh_hmapheader(map_ptr[0]);
    for(size_t bucket_index = 0; bucket_index < map_hdr->bucket_count; ++bucket_index) {
        hh_darrfree(map_hdr->buckets[bucket_index]);
    }
    free(map_hdr->buckets);
    hh_darrfree(map_hdr->bucket_lookup);
    free(map_hdr);
    *map_ptr = NULL;
}
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
#define hmap_opt hh_hmap_opt
#define hmapconfig hh_hmapconfig
#define hmapinsert hh_hmapinsert
#define hmaplen hh_hmaplen
#define hh_hmapfree hh_hh_hmapfree
#define hmapget hh_hmapget
#define hmapremove hh_hmapremove
//
#endif // HH_STRIP_PREFIXES
//
#endif // not HH__STRIP_PREFIXES
