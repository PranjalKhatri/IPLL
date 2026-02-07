#pragma once
// prime number helps distribution
#define TABLE_SIZE 101

// Uses chaining
typedef struct Entry {
    char* key;
    int value;
    struct Entry* next;
} Entry;

typedef struct HashTable {
    Entry* buckets[TABLE_SIZE];
} HashTable;

// create hash table
HashTable* ht_create(void);
// insert or update
void ht_put(HashTable* ht, const char* key, int value);
// lookup
int ht_get(HashTable* ht, const char* key, int* out);
// delete key
void ht_remove(HashTable* ht, const char* key);
// free table
void ht_destroy(HashTable* ht);
// used for debugging
void ht_print(HashTable* ht);
