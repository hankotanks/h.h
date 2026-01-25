#define HH_IMPLEMENTATION
#define HH_STRIP_PREFIXES
#include "h.h"

#include <stdbool.h>

typedef bool (*op_f)(map_t* map, span_t* token);

// parsing functions for commands
bool
op_insert(map_t* map, span_t* token);
bool
op_remove(map_t* map, span_t* token);
bool
op_get(map_t* map, span_t* token);

// helper functions for printing the cstr2cstr map
void
cstr2cstr_dump_keys(const map_t* map);
void
cstr2cstr_dump_buckets(const map_t* map);

// TODO: I received a super rare segfault in this example after the following
// insert: hello, world
// insert: hello, world2
// remove: hello

int
main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
    // initialize map of available commands
    map_t op_map = { .bucket_count = 5, 0 };
    map_insert_with_cstr_key(&op_map, "insert", &fp_wrap(op_insert), sizeof(fp_wrap_t));
    map_insert_with_cstr_key(&op_map, "remove", &fp_wrap(op_remove), sizeof(fp_wrap_t));
    map_insert_with_cstr_key(&op_map, "get",    &fp_wrap(op_get), sizeof(fp_wrap_t));
    // print usage
    printf("Usage:\n");
    printf("  > insert: <key>, <value>\n");
    printf("  > remove: <key>\n");
    printf("  > get: <key>\n\n");
    // initialize map (cstr -> cstr)
    map_t cstr2cstr = { .bucket_count = 8, 0 };
    // run the shell
    for(;;) { 
        printf("> ");
        // get line from stdin
        static char line[512];
        if(fgets(line, ARR_LEN(line), stdin) == NULL) {
            printf("\nExiting.\n");
            break;
        }
        size_t len = strlen(line);
        if(len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        // parse the first token (which should be the command)
        span_t token;
        if(!span_init(&token, line, ":")) {
            ERR("Failed to parse.");
            continue;
        }
        // retrieve command from hashmap
        op_f op = fp_unwrap(map_get_val(&op_map, token.ptr, token.len), op_f);
        if(op == NULL) {
            ERR("Unrecognized command: %.*s [%zu]", (int) token.len, token.ptr, token.len);
            continue;
        }
        // execute the corresponding command
        if(!(op)(&cstr2cstr, &token)) continue;
        // print current state of cstr2cstr
        cstr2cstr_dump_keys(&cstr2cstr);
        cstr2cstr_dump_buckets(&cstr2cstr);
    }
    map_free(&op_map);
    map_free(&cstr2cstr);
    return 0;
}

//
// implementations
//

bool
op_insert(map_t* map, span_t* token) {
    token->delim = ",";
    if(!span_next(token)) {
        ERR("Failed to parse token.");
        return false;
    }
    printf("key: len = %zu, token = \"%.*s\"\n", token->len, (int) token->len, token->ptr);
    span_t val = *token;
    if(!span_next(&val)) {
        ERR("Failed to parse token.");
        return false;
    }
    printf("val: len = %zu, token = \"%.*s\"\n", val.len, (int) val.len, val.ptr);
    if(!map_insert(map, token->ptr, token->len, val.ptr, val.len)) {
        ERR("Failed to insert element: (%.*s, %.*s)", (int) token->len, token->ptr, (int) val.len, val.ptr);
        return false;
    }
    return true;
}

bool
op_remove(map_t* map, span_t* token) {
    if(!span_next(token)) {
        ERR("Failed to parse token.");
        return false;
    }
    map_entry_t entry = map_get(map, token->ptr, token->len);
    if(entry.val == NULL) {
        ERR("Given key was not found: %.*s", (int) token->len, token->ptr);
        return false;
    }
    return map_remove(map, token->ptr, token->len);
}

bool
op_get(map_t* map, span_t* token) {
    if(!span_next(token)) {
        ERR("Failed to parse token.");
        return false;
    }
    map_entry_t entry = map_get(map, token->ptr, token->len);
    if(entry.val == NULL) {
        ERR("Failed to get element: %.*s", (int) token->len, token->ptr);
        return false;
    }
    printf("found: (%.*s, %.*s)\n", 
        (int) entry.size_key, (char*) entry.key, 
        (int) entry.size_val, (char*) entry.val);
    return true;
}

void
cstr2cstr_dump_keys(const map_t* map) {
    printf("keys: ");
    map_it(map, it) printf("%.*s, ", (int)it.size_key, (char*)it.key);
    printf("\n");
}

void
cstr2cstr_dump_buckets(const map_t* map) {
    printf("buckets:\n");
    map_entry_t entry;
    for(size_t i = 0; i < map->bucket_count; ++i) {
        printf("%zu: ", i);
        for(size_t j = 0; j < darrlen(map->buckets[i]);) {
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
