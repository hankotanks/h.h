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

#define BUF_LEN 512

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
        static char buf[BUF_LEN];
        if(fgets(buf, ARR_LEN(buf), stdin) == NULL) {
            printf("\nExiting.\n");
            break;
        }
        size_t len = strlen(buf);
        if(len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
        span_t line = span(buf);
        // parse the first token (which should be the command)
        span_t token = span_next(&line, .delim = ":", .trim = true);
        if(token.ptr == NULL) {
            ERR("Failed to parse.");
            continue;
        }
        // retrieve command from hashmap
        op_f op = fp_unwrap(map_get_val(&op_map, token.ptr, span_len(token)), op_f);
        if(op == NULL) {
            ERR("Unrecognized command: " span_fmt " [%zu]", span_fmt_args(token), span_len(token));
            continue;
        }
        // execute the corresponding command
        if(!(op)(&cstr2cstr, &line)) continue;
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
    printf("token: len = %zu, token = \"" span_fmt "\"\n", span_len(*token), span_fmt_args(*token));
    span_t key = span_next(token, .delim = ",", .trim = true);
    if(key.ptr == NULL) {
        ERR("Failed to parse token.");
        return false;
    }
    printf("token: len = %zu, token = \"" span_fmt "\"\n", span_len(*token), span_fmt_args(*token));
    printf("key: len = %zu, token = \"" span_fmt "\"\n", span_len(key), span_fmt_args(key));
    span_t val = span_next(token, .eol = true, .trim = true);
    if(val.ptr == NULL) {
        ERR("Failed to parse token.");
        return false;
    }
    printf("val: len = %zu, token = \"" span_fmt "\"\n", span_len(val), span_fmt_args(val));
    if(!map_insert(map, key.ptr, span_len(key), val.ptr, span_len(val))) {
        ERR("Failed to insert element: (" span_fmt ", " span_fmt ")", span_fmt_args(key), span_fmt_args(val));
        return false;
    }
    return true;
}

bool
op_remove(map_t* map, span_t* token) {
    span_t key = span_next(token, .eol = true, .trim = true);
    if(key.ptr == NULL) {
        ERR("Failed to parse token.");
        return false;
    }
    map_entry_t entry = map_get(map, key.ptr, span_len(key));
    if(entry.val == NULL) {
        ERR("Given key was not found: " span_fmt, span_fmt_args(key));
        return false;
    }
    return map_remove(map, key.ptr, span_len(key));
}

bool
op_get(map_t* map, span_t* token) {
    span_t key = span_next(token, .eol = true, .trim = true);
    if(key.ptr == NULL) {
        ERR("Failed to parse token.");
        return false;
    }
    map_entry_t entry = map_get(map, key.ptr, span_len(key));
    if(entry.val == NULL) {
        ERR("Failed to get element: " span_fmt, span_fmt_args(key));
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
