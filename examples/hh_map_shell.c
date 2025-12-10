#define HH_IMPLEMENTATION
#include "hh.h"

#include <stdbool.h>

typedef bool (*op_f)(hh_map_t* map, hh_span_t* token);

// parsing functions for commands
bool
op_insert(hh_map_t* map, hh_span_t* token);
bool
op_remove(hh_map_t* map, hh_span_t* token);
bool
op_get(hh_map_t* map, hh_span_t* token);

// helper functions for printing the cstr2cstr map
void
cstr2cstr_dump_keys(const hh_map_t* map);
void
cstr2cstr_dump_buckets(const hh_map_t* map);

int
main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
    // initialize map of available commands
    hh_map_t op_map = { .bucket_count = 5, 0 };
    hh_map_insert_with_cstr_key(&op_map, "insert", &hh_fp_wrap(op_insert), sizeof(hh_fp_wrap_t));
    hh_map_insert_with_cstr_key(&op_map, "remove", &hh_fp_wrap(op_remove), sizeof(hh_fp_wrap_t));
    hh_map_insert_with_cstr_key(&op_map, "get",    &hh_fp_wrap(op_get), sizeof(hh_fp_wrap_t));
    // print usage
    printf("Usage:\n");
    printf("  > insert: <key>, <value>\n");
    printf("  > remove: <key>\n");
    printf("  > get: <key>\n\n");
    // initialize map (cstr -> cstr)
    hh_map_t cstr2cstr = { .bucket_count = 8, 0 };
    // run the shell
    for(;;) { 
        printf("> ");
        // get line from stdin
        static char line[512];
        if(fgets(line, HH_ARR_LEN(line), stdin) == NULL) {
            printf("\nExiting.\n");
            break;
        }
        size_t len = strlen(line);
        if(len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        // parse the first token (which should be the command)
        hh_span_t token;
        if(!hh_span_init(&token, line, ":")) {
            HH_ERR("Failed to parse.");
            continue;
        }
        // retrieve command from hashmap
        op_f op = hh_fp_unwrap(hh_map_get_val(&op_map, token.ptr, token.len), op_f);
        if(op == NULL) {
            HH_ERR("Unrecognized command: %.*s [%zu]", (int) token.len, token.ptr, token.len);
            continue;
        }
        // execute the corresponding command
        if(!(op)(&cstr2cstr, &token)) continue;
        // print current state of cstr2cstr
        cstr2cstr_dump_keys(&cstr2cstr);
        cstr2cstr_dump_buckets(&cstr2cstr);
    }
    hh_map_free(&op_map);
    hh_map_free(&cstr2cstr);
    return 0;
}

//
// implementations
//

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
    return hh_map_remove(map, token->ptr, token->len);
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

void
cstr2cstr_dump_keys(const hh_map_t* map) {
    printf("keys: ");
    hh_map_it(map, it) printf("%.*s, ", (int)it.size_key, (char*)it.key);
    printf("\n");
}

void
cstr2cstr_dump_buckets(const hh_map_t* map) {
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
