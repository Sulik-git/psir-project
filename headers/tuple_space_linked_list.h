#ifndef TUPLE_SPACE_LINKED_LIST_H
#define TUPLE_SPACE_LINKED_LIST_H
#include "../headers/tuple_space_API.h"

typedef struct tuple_list_node
{

    tuple_t tuple;
    struct tuple_list_node *next_tuple;

} tuple_list_node_t;

void add_to_space( tuple_t data );
void remove_from_space( tuple_t tuple_template, tuple_t *retrive_tuple );
void retrive_from_space( tuple_t tuple_template, tuple_t *retrive_tuple, int *rdp_Result );

#endif