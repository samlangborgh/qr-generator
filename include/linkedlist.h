#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct linkedlistNode{
    unsigned char data;
    struct linkedlistNode* next;
} linkedlistNode;

typedef struct linkedlist{
    struct linkedlistNode* head;
    struct linkedlistNode* tail;
    size_t size;
} linkedlist;

linkedlist* createLinkedList();
void freeLinkedList(linkedlist* list);
void addLinkedListNode(linkedlist* list, unsigned char data);
unsigned char getNodeData(linkedlist* list, unsigned int pos);

#endif
