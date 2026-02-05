#include "list.h"

#include <stdlib.h>

LinkedList *list_create(void) {
    LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
    if (!list) return NULL;

    list->head = NULL;
    list->size = 0;
    return list;
}

void list_push_front(LinkedList *list, void *data) {
    ListNode *node;

    if (!list) return;

    node = (ListNode *)malloc(sizeof(ListNode));
    if (!node) return;

    node->data = data;
    node->next = list->head;
    list->head = node;
    list->size++;
}

void list_push_back(LinkedList *list, void *data) {
    ListNode *node;
    ListNode *cur;

    if (!list) return;

    node = (ListNode *)malloc(sizeof(ListNode));
    if (!node) return;

    node->data = data;
    node->next = NULL;

    if (!list->head) {
        list->head = node;
    } else {
        cur = list->head;
        while (cur->next)
            cur = cur->next;
        cur->next = node;
    }

    list->size++;
}

void *list_pop_front(LinkedList *list) {
    ListNode *node;
    void *data;

    if (!list || !list->head) return NULL;

    node = list->head;
    data = node->data;

    list->head = node->next;
    free(node);
    list->size--;

    return data;
}

void list_clear(LinkedList *list, void (*free_fn)(void *)) {
    ListNode *cur;
    ListNode *next;

    if (!list) return;

    cur = list->head;
    while (cur) {
        next = cur->next;
        if (free_fn)
            free_fn(cur->data);
        free(cur);
        cur = next;
    }

    list->head = NULL;
    list->size = 0;
}

void list_destroy(LinkedList *list, void (*free_fn)(void *)) {
    if (!list) return;
    list_clear(list, free_fn);
    free(list);
}
