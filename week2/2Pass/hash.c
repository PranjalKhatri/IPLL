#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// djb2 string hash
unsigned long hash(const char* str) {
    unsigned long h = 5381;
    int c;

    while ((c = *str++)) h = ((h << 5) + h) + c; /* h * 33 + c */

    return h % TABLE_SIZE;
}

// create hash table
HashTable* ht_create(void) {
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    int i;

    if (!ht) return NULL;

    for (i = 0; i < TABLE_SIZE; i++) ht->buckets[i] = NULL;

    return ht;
}

// insert or update
void ht_put(HashTable* ht, const char* key, int value) {
    unsigned long idx = hash(key);
    Entry* e          = ht->buckets[idx];

    // update if key exists
    while (e) {
        if (strcmp(e->key, key) == 0) {
            e->value = value;
            return;
        }
        e = e->next;
    }

    // insert new entry
    e = (Entry*)malloc(sizeof(Entry));
    if (!e) {
        printf("Out of memory\n");
        exit(2);
    }

    e->key = malloc(strlen(key) + 1); /* +1 for '\0' */
    if (!e->key) {
        printf("Out of memory in hash table put while inserting %s\n", key);
        exit(2);
    }

    strcpy(e->key, key);
    e->value         = value;
    e->next          = ht->buckets[idx];
    ht->buckets[idx] = e;
}

// lookup
int ht_get(HashTable* ht, const char* key, int* out) {
    unsigned long idx = hash(key);
    Entry* e          = ht->buckets[idx];

    while (e) {
        if (strcmp(e->key, key) == 0) {
            *out = e->value;
            return 1; /* found */
        }
        e = e->next;
    }

    return 0; /* not found */
}

// delete key
void ht_remove(HashTable* ht, const char* key) {
    unsigned long idx = hash(key);
    Entry* e          = ht->buckets[idx];
    Entry* prev       = NULL;

    while (e) {
        if (strcmp(e->key, key) == 0) {
            if (prev)
                prev->next = e->next;
            else
                ht->buckets[idx] = e->next;

            free(e->key);
            free(e);
            return;
        }
        prev = e;
        e    = e->next;
    }
}

// free table
void ht_destroy(HashTable* ht) {
    int i;
    Entry *e, *next;

    for (i = 0; i < TABLE_SIZE; i++) {
        e = ht->buckets[i];
        while (e) {
            next = e->next;
            free(e->key);
            free(e);
            e = next;
        }
    }
    free(ht);
}
// debugging
void ht_print(HashTable* ht) {
    int i;
    Entry* e;

    for (i = 0; i < TABLE_SIZE; i++) {
        if (ht->buckets[i] == NULL) continue;

        printf("[%d]: ", i);
        e = ht->buckets[i];

        while (e) {
            printf("(%s -> %d)", e->key, e->value);
            if (e->next) printf(" -> ");
            e = e->next;
        }
        printf("\n");
    }
}
