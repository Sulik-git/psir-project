#include "../headers/tuple_space_linked_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TUPLE_LEN_CHECK_PHASE 0
#define RESET_CHECK_PHASE 0
#define TUPLE_NAME_CHECK_PHASE 1
#define TUPLE_FIELDS_CHECK_PHASE 2


tuple_list_node_t *head = NULL;
tuple_t empty_tuple;
int check_phase = RESET_CHECK_PHASE;


void print_tuple( tuple_t tuple_to_print )
{

    for ( int i = 0; i < tuple_to_print.tuple_len; i++ )
    {

        if ( tuple_to_print.tuple_fields[i].type == TS_INT )
        {

            printf( "field %d: %d ", i + 1, tuple_to_print.tuple_fields[i].data.int_field );

        }
        else
        {

            printf( "field %d: %f ", i + 1, tuple_to_print.tuple_fields[i].data.float_field );

        }
    }

    printf( "\n" );

}

void add_to_space(tuple_t data)
{

    //Allocating memory for new node and copying data to this node
    tuple_list_node_t *n = (tuple_list_node_t *) calloc( 1, sizeof(tuple_list_node_t) );
    n->tuple = data;


    tuple_list_node_t *linked_list = head;
    if ( linked_list != NULL )
    {
                                                                        //Goes here if linked list is not empty
        while ( linked_list->next_tuple != NULL )
            linked_list = linked_list->next_tuple;


        linked_list->next_tuple = n;
        printf( "SERVER: Added tuple - name: %s ", n->tuple.name );
        print_tuple( n->tuple );

    }
    else
    {
                                                                        //Goes here if linked list is empty
        head = n;
        head->next_tuple = NULL;
        printf( "SERVER: Added tuple - name: %s ", n->tuple.name );
        print_tuple( n->tuple );

    }
}

void delete_at_index( int index )
{

    int i = 0;
    int retval = -1;
    tuple_list_node_t *current = head;
    tuple_list_node_t *temp_node = NULL;


    if ( index == 0 && head != NULL )
    {
        if ( current->next_tuple == NULL )
        {

            free( head );
            return;

        }
        else
        {

            temp_node = head->next_tuple;
            free( head );
            head = temp_node;
            return;

        }
    }


    for ( i = 0; i < index - 1; i++ )
    {
        if ( current->next_tuple == NULL )
        {

            return;

        }
        current = current->next_tuple;

    }

    if ( current->next_tuple == NULL )
    {

        return;

    }


    temp_node = current->next_tuple;
    current->next_tuple = temp_node->next_tuple;
    free( temp_node );

}

int check_for_tuple( tuple_t template, tuple_t temp_tuple )
{

    if ( check_phase == TUPLE_LEN_CHECK_PHASE )
    {
        if ( template.tuple_len == temp_tuple.tuple_len )
        {

            check_phase++;
            check_for_tuple( template, temp_tuple );

        }
        else
        {
            
            check_phase = RESET_CHECK_PHASE;
            return TS_FAILURE;

        }
    }
    else if ( check_phase == TUPLE_NAME_CHECK_PHASE )
    {
        if ( strcmp(template.name, temp_tuple.name) == 0 )
        {

            check_phase++;
            check_for_tuple( template, temp_tuple );

        }
        else
        {

            check_phase = RESET_CHECK_PHASE;
            return TS_FAILURE;

        }
    }
    else if ( template.tuple_len > 0 && check_phase == TUPLE_FIELDS_CHECK_PHASE )
    {

        if ( fieldcmp(template.tuple_fields[template.tuple_len - 1], temp_tuple.tuple_fields[template.tuple_len - 1]) )
        {

            template.tuple_len--;
            check_for_tuple(template, temp_tuple);

        }
        else
        {

            check_phase = RESET_CHECK_PHASE;
            return TS_FAILURE;

        }
    }
    else if ( template.tuple_len == 0 && check_phase == TUPLE_FIELDS_CHECK_PHASE )
    {

        check_phase = RESET_CHECK_PHASE;
        return TS_SUCCESS;

    }
}

void remove_from_space( tuple_t tuple_template, tuple_t *retrive_tuple )
{

    int index = 0;
    *retrive_tuple = empty_tuple;
    if ( head == NULL )
    {

        printf("SERVER ERROR: Cannot remove tuple because tuple space is empty\n");
        return;

    }

    for ( tuple_list_node_t *i = head; i != NULL; i = i->next_tuple )
    {

        if ( idcmp(tuple_template.tuple_fields[0], i->tuple.tuple_fields[0]) )
        {

            if ( check_for_tuple(tuple_template, i->tuple) )
            {

                *retrive_tuple = i->tuple;
                delete_at_index( index );
                break;

            }
        }


        index++;

    }

    // Checks whether ID is_actual is equal to 0 which indicates that retrived tuple is empty 
    if( retrive_tuple->tuple_fields[0].is_actual == 0 )
    {

        printf( "SERVER: Given tuple does not exist in tuple space\n" );

    }
    else
    {

        printf( "SERVER: Removed tuple from Tuple Space\n" );

    }

}

void retrive_from_space( tuple_t tuple_template, tuple_t *retrive_tuple, int *rdp_result )
{

    *retrive_tuple = empty_tuple;
    if ( head == NULL )
    {

        printf( "SERVER ERROR: Cannot retrive tuple because tuple space is empty\n" );
        return;

    }


    *rdp_result = TS_FAILURE;
    for ( tuple_list_node_t *i = head; i != NULL; i = i->next_tuple )
    {
        if ( idcmp(tuple_template.tuple_fields[0], i->tuple.tuple_fields[0]) )
        {
            if ( check_for_tuple(tuple_template, i->tuple) )
            {

                *retrive_tuple = i->tuple;
                *rdp_result = TS_SUCCESS;
                break;

            }
        }
    }

    // Checks whether ID is_actual is equal to 0 which indicates that retrived tuple is empty 
    if( retrive_tuple->tuple_fields[0].is_actual == 0 )
    {

        printf( "SERVER: Retrived empty tuple from Tuple Space\n" );

    }
    else
    {

        printf( "SERVER: Retrived tuple from Tuple Space\n" );

    }

}
