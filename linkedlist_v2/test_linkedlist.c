#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"

extern struct Node *head_list;

void test_linkedlist();

void print_list();

int main() {

    test_linkedlist();

    return (0);

} /* end of main */

void test_linkedlist() {

    struct Node *node;

    /* Test adding to empty list */
    node = malloc(sizeof(struct Node));
    node->payload = 5;
    add_to_list(node);
    printf("Expected output: 5,\n");
    print_list();

    /* Test deleting from an empty list */
    delete_from_list_by_key(5);
    printf("Expected output: (empty)\n");
    print_list();

    /* Test deleting from empty list - no error */
    delete_from_list_by_key(5);
    printf("Expected output: (empty)\n");
    print_list();

    /* Add several nodes */
    for (int i = 1; i <= 4; i++) {
        node = malloc(sizeof(struct Node));
        node->payload = i;
        add_to_list(node);
    }
    printf("Expected output: 4, 3, 2, 1,\n");
    print_list();

    /* Test searching for existing node */
    node = search_list_by_key(3);
    printf("Expected output: 3, not NULL\n");
    printf("%d, %p\n", node->payload, node);

    /* Test searching for non-existent node */
    node = search_list_by_key(0);
    printf("Expected output: NULL pointer\n");
    printf("%p\n", node);

    /* Test deleting from list */
    delete_from_list_by_key(2);
    printf("Expected output: 4, 3, 1,\n");
    print_list();

    cleanup();

} /* end of test linkedlist */

void print_list() {

    struct Node *current = head_list;
    while (current != NULL) {
        printf("%d, ", current->payload);
        current = current->next;
    }
    printf("\n");

} /* end of print_linkedlist */
