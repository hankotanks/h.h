#include "h.h"
#include "hmap.h"
#include <stddef.h>
#include <string.h>

size_t
hh_hash_cstr(const void* ptr) {
    const char* str = *((const char**) ptr);
    return HH__hash_djb2(str, strlen(str));
}

int
hh_comp_cstr(const void* fst, const void* snd) {
    return strcmp(*((const char**) fst), *((const char**) snd));
}

char* randstr(size_t n) {
    static const char charset[] = \
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";
    char *str = malloc(n + 1);
    if(str == NULL) return NULL;
    for(size_t i = 0; i < n; i++)
        str[i] = charset[(unsigned long) rand() % (sizeof(charset) - 1)];
    str[n] = '\0';
    return str;
}

#define MAX_SZ 500
#define MAX_KEY_SZ 1
// #define DUMP_MAP

typedef struct { const char* key; size_t val; } entry_t;

void
dump(entry_t* map) {
#ifdef DUMP_MAP
    for(size_t i = 0; i < hh_hmaplen(map); ++i) {
        printf("(%s, %zu), ", map[i].key, map[i].val);
    }
    printf("\n");
#else
    (void) map;
#endif
}

size_t
randinrange(size_t min, size_t max) {
    // [min, max)
    HH_ASSERT_INVARIANT(min < max);
    return min + (size_t) rand() % (max - min);
}

int main(void) {
    char key_temp[MAX_KEY_SZ + 1];
    char* key_temp2 = key_temp;
    entry_t* str2sz = NULL;
    hh_hmapconfig(str2sz, .comp = hh_comp_cstr, .hash = hh_hash_cstr);
    entry_t* repl;
    for(size_t i = 0, j = 0, k; i < MAX_SZ; ++i) {
        const char* key = randstr(MAX_KEY_SZ);
        repl = hh_hmapinsert(str2sz, &key, j);
        printf("[%zu] inserted: (%s, %zu)\n", hh_hmaplen(str2sz), key, j);
        dump(str2sz);
        j++;
        if(repl != NULL) {
            printf("[%zu] replaced: (%s, %zu) with (%s, %zu)\n", hh_hmaplen(str2sz), repl->key, repl->val, key, j - 1);
            dump(str2sz);
            free((char*) repl->key);
        }
        if(!randinrange(0, 2) && hh_hmaplen(str2sz) > 0) {
            k = randinrange(0, hh_hmaplen(str2sz));
            strcpy(key_temp, str2sz[k].key);
            repl = hh_hmapremove(str2sz, &key_temp2);
            printf("[%zu] removed: (%s, %zu)\n", hh_hmaplen(str2sz), repl->key, repl->val);
            dump(str2sz);
            free((char*) repl->key);
        }
    }

    for(size_t i; hh_hmaplen(str2sz) > 0;) {
        i = randinrange(0, hh_hmaplen(str2sz));
        strcpy(key_temp, str2sz[i].key);
        repl = hh_hmapremove(str2sz, &key_temp2);
        printf("removed: (%s, %zu)\n", repl->key, repl->val);
        dump(str2sz);
        free((char*) repl->key);
    }

    hh_hmapfree(str2sz);

#if 0
    struct { char key; int val; }* ch2int = NULL;
    char ch_temp;
    ch_temp = 'a'; hh_hmapinsert(ch2int, &ch_temp, 14);
    ch_temp = 'b'; hh_hmapinsert(ch2int, &ch_temp, 32);
    for(size_t i = 0; i < hh_hmaplen(ch2int); ++i) {
        HH_MSG("(%c, %d)", ch2int[i].key, ch2int[i].val);
    }
    ch_temp = 'a'; hh_hmapinsert(ch2int, &ch_temp, 16);
    size_t a = hh_hmapget(ch2int, &ch_temp);
    HH_MSG("(%c, %d)", ch2int[a].key, ch2int[a].val);
#endif
#if 0
    struct { const char* key; size_t val; }* str2sz = NULL;
    hh_hmapconfig(str2sz, .comp = strcmp, .hash = hh_hash_cstr);
    const char* str_temp;
    str_temp = "hello"; hh_hmapinsert(str2sz, &str_temp, 14);
    str_temp = "world"; 
    hh_hmapinsert(str2sz, &str_temp, 32);
    for(size_t i = 0; i < hh_hmaplen(str2sz); ++i) {
        HH_MSG("(%s, %zu)", str2sz[i].key, str2sz[i].val);
    }
    str_temp = "hello"; hh_hmapinsert(str2sz, &str_temp, 16);
    str_temp = "hello"; 
    size_t hello = hh_hmapget(str2sz, &str_temp);
    HH_MSG("(%s, %zu)", str2sz[hello].key, str2sz[hello].val);
#endif
    return 0;
}
