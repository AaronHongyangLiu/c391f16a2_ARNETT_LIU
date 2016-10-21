#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for memset */
#include <assert.h>
#include "linkedlist.h"

struct Node *head_list = NULL; /* Definition */

void initialize_linked_list(struct Node *head) {

    head_list = head;

} /* end of initialize_linked_list */

void add_to_list(struct Node *node) {

    node->next = head_list;
    head_list = node;

} /* end of add_to_list */

void delete_from_list_by_key(int key) {

    struct Node *prev = NULL;
    struct Node *current = head_list;

    if (head_list == NULL) return;

    while (current != NULL) {
        if (current->payload == key) {
            break;
        }
        prev = current;
        current = current->next;
    }

    if (prev == NULL) {
        head_list = current->next;
    } else {
        prev->next = current->next;
    }
    if (current != NULL) {
        free(current);
    }


} /*end of delete_from_list_by_key */

struct Node *search_list_by_key(int key) {

    struct Node *current = head_list;

    while (current != NULL) {
        if (current->payload == key) {
            return current;
        }
        current = current->next;
    }

    return NULL;

} /* end of search_list_by_key */

void cleanup(void) {

    struct Node *current = head_list;
    struct Node *temp = NULL;

    while (current != NULL) {
        temp = current->next;
        free(current);
        current = temp;
    }
} /* cleanup */
