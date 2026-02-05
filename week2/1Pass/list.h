#pragma once

#include <stddef.h>

// Singly linked list node
typedef struct ListNode {
    void* data;
    struct ListNode* next;
} ListNode;

// Linked list handle
typedef struct LinkedList {
    ListNode* head;
    size_t size;
} LinkedList;

// API
LinkedList* list_create(void);
void list_push_front(LinkedList* list, void* data);
void list_push_back(LinkedList* list, void* data);
void* list_pop_front(LinkedList* list);
void list_clear(LinkedList* list, void (*free_fn)(void*));
void list_destroy(LinkedList* list, void (*free_fn)(void*));
