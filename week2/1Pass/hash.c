#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned hash(const char* s) {
    unsigned h = 0;
    while (*s) h = h * 31 + (unsigned char)*s++;
    return h % TABLE_SIZE;
}

// create hash table
HashTable* ht_create(void) {
    HashTable* ht = malloc(sizeof(HashTable));
    if (!ht) return NULL;

    for (int i = 0; i < TABLE_SIZE; i++) ht->buckets[i] = NULL;

    return ht;
}

Entry* ht_find(HashTable* ht, const char* key) {
    unsigned idx = hash(key);
    Entry* e     = ht->buckets[idx];

    while (e) {
        if (strcmp(e->key, key) == 0) return e;
        e = e->next;
    }
    return NULL;
}

Entry* ht_find_or_create(HashTable* ht, const char* key) {
    unsigned idx = hash(key);
    Entry* e     = ht->buckets[idx];

    while (e) {
        if (strcmp(e->key, key) == 0) return e;
        e = e->next;
    }

    // create new entry
    e                = malloc(sizeof(Entry));
    e->key           = strdup(key);
    e->value         = 0;
    e->defined       = 0;
    e->backrefs      = NULL;

    e->next          = ht->buckets[idx];
    ht->buckets[idx] = e;

    return e;
}
// insert or update
void ht_put(HashTable* ht, const char* key, int value) {
    Entry* e   = ht_find_or_create(ht, key);
    e->value   = value;
    e->defined = 1;
}

// lookup
int ht_get(HashTable* ht, const char* key, int* out) {
    Entry* e = ht_find(ht, key);
    if (!e || !e->defined) return 0;

    *out = e->value;
    return 1;
}

void ht_add_backref(HashTable* ht, const char* key, long file_offset,int useIndexed) {
    Entry* e         = ht_find_or_create(ht, key);

    BackRef* ref     = malloc(sizeof(BackRef));
    ref->indexed = useIndexed;
    ref->file_offset = file_offset;
    ref->next        = e->backrefs;
    e->backrefs      = ref;
}

BackRef* ht_get_backrefs(HashTable* ht, const char* key) {
    Entry* e = ht_find(ht, key);
    return e ? e->backrefs : NULL;
}

void ht_clear_backrefs(HashTable* ht, const char* key) {
    Entry* e = ht_find(ht, key);
    if (!e) return;

    BackRef* ref = e->backrefs;
    while (ref) {
        BackRef* tmp = ref;
        ref          = ref->next;
        free(tmp);
    }
    e->backrefs = NULL;
}
// delete key
void ht_remove(HashTable* ht, const char* key) {
    unsigned idx = hash(key);
    Entry* prev  = NULL;
    Entry* e     = ht->buckets[idx];

    while (e) {
        if (strcmp(e->key, key) == 0) {
            if (prev)
                prev->next = e->next;
            else
                ht->buckets[idx] = e->next;

            ht_clear_backrefs(ht, key);
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
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry* e = ht->buckets[i];
        while (e) {
            Entry* tmp   = e;
            e            = e->next;

            BackRef* ref = tmp->backrefs;
            while (ref) {
                BackRef* r = ref;
                ref        = ref->next;
                free(r);
            }

            free(tmp->key);
            free(tmp);
        }
    }
    free(ht);
}
// debugging
void ht_print(HashTable* ht) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry* e = ht->buckets[i];
        if (!e) continue;

        printf("[%d]:\n", i);
        while (e) {
            printf("  %s = %d (%s)\n", e->key, e->value,
                   e->defined ? "defined" : "undef");

            BackRef* r = e->backrefs;
            while (r) {
                printf("    backref @ %ld\n", r->file_offset);
                r = r->next;
            }
            e = e->next;
        }
    }
}

int ht_check_unresolved(HashTable* ht) {
    int errors = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry* e = ht->buckets[i];
        while (e) {
            if (!e->defined && e->backrefs) {
                fprintf(stderr, "Error: Undefined symbol '%s'\n", e->key);

                BackRef* r = e->backrefs;
                while (r) {
                    fprintf(stderr, "  referenced at file offset %ld\n",
                            r->file_offset);
                    r = r->next;
                }

                errors++;
            }
            e = e->next;
        }
    }

    return errors;  // number of unresolved symbols
}
