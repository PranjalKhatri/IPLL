#pragma once
#include <stddef.h>

// prime number helps distribution
#define TABLE_SIZE 101

typedef struct BackRef {
    long file_offset;  // where to patch
    int indexed;  // Whether the original instruction used ,X addressing mode
    struct BackRef* next;
} BackRef;

// Uses chaining
typedef struct Entry {
    char* key;           // label name
    int value;           // label address
    int defined;         // 0 = undefined, 1 = defined
    BackRef* backrefs;   // backreference chain
    struct Entry* next;  // hash collision chain
} Entry;

typedef struct HashTable {
    Entry* buckets[TABLE_SIZE];
} HashTable;

// core API
HashTable* ht_create(void);
void ht_put(HashTable* ht, const char* key, int value);
int ht_get(HashTable* ht, const char* key, int* out);
void ht_remove(HashTable* ht, const char* key);
void ht_destroy(HashTable* ht);
void ht_print(HashTable* ht);

// backreference API
void ht_add_backref(HashTable* ht, const char* key, long file_offset,
                    int useIndexed);
BackRef* ht_get_backrefs(HashTable* ht, const char* key);
void ht_clear_backrefs(HashTable* ht, const char* key);

int ht_check_unresolved(HashTable* ht);
Entry* ht_find(HashTable* ht, const char* key);
Entry* ht_find_or_create(HashTable* ht, const char* key);
