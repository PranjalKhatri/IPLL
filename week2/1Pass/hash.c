#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* djb2 */
static unsigned long hash(const char *str) {
    unsigned long h = 5381;
    int c;

    while ((c = *str++))
        h = ((h << 5) + h) + c;

    return h % TABLE_SIZE;
}

HashTable *ht_create(void) {
    HashTable *ht;
    int i;

    ht = (HashTable *)malloc(sizeof(HashTable));
    if (!ht) return NULL;

    for (i = 0; i < TABLE_SIZE; i++)
        ht->buckets[i] = NULL;

    return ht;
}

void ht_put(HashTable *ht, const char *key, void *value) {
    unsigned long idx;
    Entry *e;

    if (!ht || !key) return;

    idx = hash(key);
    e = ht->buckets[idx];

    /* update */
    while (e) {
        if (strcmp(e->key, key) == 0) {
            e->value = value;
            return;
        }
        e = e->next;
    }

    /* insert */
    e = (Entry *)malloc(sizeof(Entry));
    if (!e) exit(2);

    e->key = (char *)malloc(strlen(key) + 1);
    if (!e->key) exit(2);

    strcpy(e->key, key);
    e->value = value;
    e->next = ht->buckets[idx];
    ht->buckets[idx] = e;
}

int ht_get(HashTable *ht, const char *key, void **out) {
    unsigned long idx;
    Entry *e;

    if (!ht || !key) return 0;

    idx = hash(key);
    e = ht->buckets[idx];

    while (e) {
        if (strcmp(e->key, key) == 0) {
            if (out) *out = e->value;
            return 1;
        }
        e = e->next;
    }
    return 0;
}

void ht_remove(HashTable *ht, const char *key, void (*free_value)(void *)) {
    unsigned long idx;
    Entry *e;
    Entry *prev;

    if (!ht || !key) return;

    idx = hash(key);
    e = ht->buckets[idx];
    prev = NULL;

    while (e) {
        if (strcmp(e->key, key) == 0) {
            if (prev)
                prev->next = e->next;
            else
                ht->buckets[idx] = e->next;

            if (free_value)
                free_value(e->value);
            free(e->key);
            free(e);
            return;
        }
        prev = e;
        e = e->next;
    }
}

void ht_destroy(HashTable *ht, void (*free_value)(void *)) {
    int i;
    Entry *e;
    Entry *next;

    if (!ht) return;

    for (i = 0; i < TABLE_SIZE; i++) {
        e = ht->buckets[i];
        while (e) {
            next = e->next;
            if (free_value)
                free_value(e->value);
            free(e->key);
            free(e);
            e = next;
        }
    }
    free(ht);
}

void ht_print(HashTable *ht, void (*print_value)(void *)) {
    int i;
    Entry *e;

    for (i = 0; i < TABLE_SIZE; i++) {
        if (!ht->buckets[i]) continue;

        printf("[%d]: ", i);
        e = ht->buckets[i];
        while (e) {
            printf("(%s -> ", e->key);
            if (print_value)
                print_value(e->value);
            else
                printf("%p", e->value);
            printf(")");
            if (e->next) printf(" -> ");
            e = e->next;
        }
        printf("\n");
    }
}
