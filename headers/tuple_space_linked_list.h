#ifndef TUPLE_SPACE_LINKED_LIST_H
#define TUPLE_SPACE_LINKED_LIST_H
#include "../headers/tuple_space_API.h"

typedef struct tuple_list_node
{

    tuple_t tuple;
    struct tuple_list_node *next_tuple;

} tuple_list_node_t;                    // Linked list node with tuple as data

void add_to_space( tuple_t data );                                                          // Adds tuple to tuple_space
void remove_from_space( tuple_t tuple_template, tuple_t *retrive_tuple, int *inp_result ); // Retrives and removes tuple from tuple space
void retrive_from_space( tuple_t tuple_template, tuple_t *retrive_tuple, int *rdp_Result ); // Retrives tuple from tuple space

#endif