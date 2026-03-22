#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HashTable *create_table(int size, unsigned int (*h)(void *),
                        int (*c)(void *, void *), void (*p)(void *, void *)) {
  HashTable *table = (HashTable *)malloc(sizeof(HashTable));
  table->size = size;
  table->hash_func = h;
  table->compare_func = c;
  table->print_func = p;
  table->buckets = (Node **)calloc(size, sizeof(Node *));
  return table;
}
void insert(HashTable *table, void *key, void *value) {
  unsigned int index = table->hash_func(key) % table->size;
  Node *new_node = (Node *)malloc(sizeof(Node));
  new_node->key = key;
  new_node->value = value;

  // Insert at the head of the list (O(1) insertion)
  new_node->next = table->buckets[index];
  table->buckets[index] = new_node;
}

void *lookup(HashTable *table, void *key) {
  unsigned int index = table->hash_func(key) % table->size;
  Node *curr = table->buckets[index];
  while (curr) {
    if (table->compare_func(curr->key, key) == 0)
      return curr->value;
    curr = curr->next;
  }
  return NULL;
}
void print_table(HashTable *table) {
    printf("Contents of hash table:\n");
  for (int i = 0; i < table->size; i++) {
    Node *curr = table->buckets[i];
    if (curr) {
      printf("Bucket %d: ", i);
      while (curr) {
        table->print_func(curr->key, curr->value);
        printf(" -> ");
        curr = curr->next;
      }
      printf("\n");
    }
  }
}

void free_table(HashTable *table) {
  for (int i = 0; i < table->size; i++) {
    Node *curr = table->buckets[i];
    while (curr) {
      Node *temp = curr;
      curr = curr->next;
      free(temp);
    }
  }
  free(table->buckets);
  free(table);
}
void free_table_with_cb(HashTable *table, void (*free_fn)(void *, void *)) {
  for (int i = 0; i < table->size; i++) {
    Node *curr = table->buckets[i];
    while (curr) {
      Node *temp = curr;
      curr = curr->next;

      if (free_fn) {
        free_fn(temp->key, temp->value);
      }

      free(temp);
    }
  }
  free(table->buckets);
  free(table);
}
