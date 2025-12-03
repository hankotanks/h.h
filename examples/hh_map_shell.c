#include "hh.h"
#include <stdbool.h>

void
cstr2cstr_dump(const hh_map_t* map) {
    printf("buckets:\n");
    hh_map_entry_t entry;
    for(size_t i = 0; i < map->bucket_count; ++i) {
        printf("%zu: ", i);
        for(size_t j = 0; j < hh_darrlen(map->buckets[i]);) {
            entry.size_key = *((size_t*) (map->buckets[i] + j)); j += sizeof(size_t);
            if(entry.size_key == 0) break;
            entry.size_val = *((size_t*) (map->buckets[i] + j)); j += sizeof(size_t);
            entry.key = map->buckets[i] + j; j += entry.size_key;
            entry.val = map->buckets[i] + j; j += entry.size_val;
            printf("(%.*s, %.*s), ", (int) entry.size_key, (char*) entry.key, (int) entry.size_val, (char*) entry.val);
		}
		printf("\n");
	}
}

//
//
//

#define BUFFER_SIZE 512

union op_ptr {
    const void* ptr;
    bool (*fp)(hh_map_t* map, hh_span_t* token);
};

#define OP(_fp) (&(union op_ptr) { .fp = _fp })

bool
op_insert(hh_map_t* map, hh_span_t* token) {
    token->delim = ",";
    if(!hh_span_next(token)) {
        HH_ERR("Failed to parse token.");
        return false;
    }
    printf("key: len = %zu, token = \"%.*s\"\n", token->len, (int) token->len, token->ptr);
    hh_span_t val = *token;
    if(!hh_span_next(&val)) {
        HH_ERR("Failed to parse token.");
        return false;
    }
    printf("val: len = %zu, token = \"%.*s\"\n", val.len, (int) val.len, val.ptr);
    if(!hh_map_insert(map, token->ptr, token->len, val.ptr, val.len)) {
        HH_ERR("Failed to insert element: (%.*s, %.*s)", (int) token->len, token->ptr, (int) val.len, val.ptr);
        return false;
    }
    return true;
}

bool
op_remove(hh_map_t* map, hh_span_t* token) {
    if(!hh_span_next(token)) {
        HH_ERR("Failed to parse token.");
        return false;
    }
    hh_map_entry_t entry = hh_map_get(map, token->ptr, token->len);
    if(entry.val == NULL) {
        HH_ERR("Given key was not found: %.*s", (int) token->len, token->ptr);
        return false;
    }
    char* key = strndup(entry.key, entry.size_key);
    char* val = strndup(entry.val, entry.size_val);
    bool ret = hh_map_remove(map, token->ptr, token->len);
    free(key);
    free(val);
    return ret;
}

bool
op_get(hh_map_t* map, hh_span_t* token) {
    if(!hh_span_next(token)) {
        HH_ERR("Failed to parse token.");
        return false;
    }
    hh_map_entry_t entry = hh_map_get(map, token->ptr, token->len);
    if(entry.val == NULL) {
        HH_ERR("Failed to get element: %.*s", (int) token->len, token->ptr);
        return false;
    }
    printf("found: (%.*s, %.*s)\n", 
        (int) entry.size_key, (char*) entry.key, 
        (int) entry.size_val, (char*) entry.val);
    return true;
}

size_t
op_man_hash(const void* key, size_t size_key) {
    if(((char*) key)[size_key - 1] == '\0') size_key--;
    size_t hash = 5381;
    for (size_t i = 0; i < size_key; ++i) hash = ((hash << 5) + hash) + (size_t) ((char*) key)[i];
    return hash;
}

int 
op_map_comp(const void* key_query, size_t size_key_query, const void* key_in, size_t size_key_in) {
    size_key_in -= 1;
    int result = memcmp(key_query, key_in, HH_MIN(size_key_query, size_key_in));
	if(result != 0) return result;
	if(size_key_query < size_key_in) return -1;
    if(size_key_query > size_key_in) return 1;
	return 0;
}

int
main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
    hh_map_t op_map = { .bucket_count = 5, .hash = op_man_hash, .comp = op_map_comp, 0 };
    hh_map_insert_with_cstr_key(&op_map, "insert", OP(op_insert), sizeof(union op_ptr));
    hh_map_insert_with_cstr_key(&op_map, "remove", OP(op_remove), sizeof(union op_ptr));
    hh_map_insert_with_cstr_key(&op_map, "get",    OP(op_get), sizeof(union op_ptr));
    printf("Usage:\n");
    printf("  > insert: <key>, <value>\n");
    printf("  > remove: <key>\n");
    printf("  > get: <key>\n\n");
    hh_map_t cstr2cstr = { .bucket_count = 8, 0 };
    char line[BUFFER_SIZE];
    hh_span_t token;
    union op_ptr* op;
    for(;;) {
        printf("> ");
        if(fgets(line, BUFFER_SIZE, stdin) == NULL) {
            printf("\nExiting.\n");
            break;
        }
        size_t len = strlen(line);
        if(len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        if(!hh_span_init(&token, line, ":")) {
            HH_ERR("Failed to parse.");
            continue;
        }
        op = (union op_ptr*) hh_map_get_val(&op_map, token.ptr, token.len);
        if(op == NULL) {
            HH_ERR("Unrecognized command: %.*s [%zu]", (int) token.len, token.ptr, token.len);
            continue;
        }
        if(!(op->fp)(&cstr2cstr, &token)) continue;
        printf("keys: ");
        hh_map_it(&cstr2cstr, it) printf("%.*s, ", (int)it.size_key, (char*)it.key);
        printf("\n");
        cstr2cstr_dump(&cstr2cstr);
        
    }
    hh_map_free(&cstr2cstr);
    return 0;
}
