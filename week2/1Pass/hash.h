#ifndef HASH_H
#define HASH_H

#define TABLE_SIZE 101

typedef struct Entry {
    char* key;
    void* value;
    struct Entry* next;
} Entry;

typedef struct HashTable {
    Entry* buckets[TABLE_SIZE];
} HashTable;

/* API */
HashTable* ht_create(void);
void ht_put(HashTable* ht, const char* key, void* value);
int ht_get(HashTable* ht, const char* key, void** out);
void ht_remove(HashTable* ht, const char* key, void (*free_value)(void*));
void ht_destroy(HashTable* ht, void (*free_value)(void*));
void ht_print(HashTable* ht, void (*print_value)(void*));

#endif /* HASH_H */
