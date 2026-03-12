#include "h.h"
#include "hmap.h"

void
dump(size_t* str2sz) {
    for(size_t i = 0; i < hh_hmaplen(str2sz); ++i) {
        printf("%zu, ", str2sz[i]);
    }
    printf("\n");
}

void
remove_entry(size_t* str2sz, char* key) {
    size_t* val = hh_hmapremove(str2sz, key, strlen(key));
    HH_ASSERT(val != NULL, "failed to remove: \"%s\"", key);
    printf("removed: (%s, %zu) [len = %zu]\n", key, *val, hh_hmaplen(str2sz));
    dump(str2sz);
}

int
main(void) {
    size_t* str2sz = NULL;

    static const size_t max_key_len = sizeof(size_t);
    static const size_t elem_to_add = 100;

    size_t* keys = NULL;

    size_t key_temp;
    char buf[max_key_len];
    for(size_t i = 0, j, k; i < elem_to_add; ++i) {
        j = (size_t) (1 + rand() % 6);
        buf[0] = (char) i + 'A';
        for(k = 1; k < j; ++k) {
            buf[k] = (char) (48 + rand() % 10);
        }
        buf[k] = '\0';
        memcpy(&key_temp, buf, sizeof(size_t));
        hh_darrput(keys, key_temp);
        hh_hmapinsert(str2sz, buf, j, &i);
        printf("inserted: (%.*s, %zu) [len = %zu]\n", (int) j, buf, i, hh_hmaplen(str2sz));
        dump(str2sz);
    }
    size_t i = hh_darrlen(keys);
    while(i > 0) {
        i--;
        memcpy(buf, &keys[i], sizeof(size_t));
        remove_entry(str2sz, buf);
    }
    hh_darrfree(keys);
    hh_hmapfree(str2sz);
    return 0;
}
