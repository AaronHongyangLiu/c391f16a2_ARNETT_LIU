/* The header file contains function prototypes, macro definitions,
 * anything that does NOT result in code being generated.
 * 
 * In this header file, we have a macro definition, a struct
 * definition, a variable declaration, and function prototypes.
 */

/* Prevents issues if a .h file is included in multiple .c files */
#ifndef _LINKED_LIST_H_ //means if not defined. this is a convention
#define _LINKED_LIST_H_

#define MAX_VALUE 128

struct Node {
    int payload;
    struct Node *next;
};

// IN HEADER FILES WE DO NOT WANT TO STORE OR ALLOCATE ANY SPACE WITH CODE


/* Declaration only. 
 * Here we state that the variable head_list will exist in our program
 * with type struct Node *, but we do not allocate any storage for it
 *
 * Using extern makes this a DECLARATION only, not a DEFINITION. 
 * Also makes the variable head_list visible to the whole program.
 * STORAGE FOR THIS VARIABLE WILL EXIST SOMEWHERE ELSE
 */
extern struct Node *head_list;

/* Functions are visible by default, so the use of extern here is
 * redundant, but just specifies that these functions can be used
 * anywhere in the program.
 */

extern void initialize_linked_list(struct Node *head);

extern void add_to_list(struct Node *node);

extern void delete_from_list_by_key(int key);

extern struct Node *search_list_by_key(int key);

extern void cleanup(void);

#endif /* _LINKED_LIST_H */
