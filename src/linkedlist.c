#include "linkedlist.h"

linkedlist* createLinkedList() {
    linkedlist* list = (linkedlist*)malloc(sizeof(linkedlist));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

void freeLinkedList(linkedlist* list) {
    while (list->head != NULL) {
        struct linkedlistNode* tmp = list->head;
        list->head = list->head->next;
        free(tmp);
    }

    free(list);
}

void addLinkedListNode(linkedlist* list, unsigned char data) {
    struct linkedlistNode* node = (linkedlistNode*)malloc(sizeof(linkedlistNode));
    node->data = data;
    node->next = NULL;

    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
}

unsigned char getNodeData(linkedlist* list, unsigned int pos) {
    assert(pos < list->size && list->head != NULL);

    struct linkedlistNode* node = list->head;

    while (pos-- > 0) {
        assert(node->next != NULL);
        node = node->next;
    }

    return node->data;
}
