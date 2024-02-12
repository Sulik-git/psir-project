#include <stdio.h>
#include "../headers/tuple_space_API.h"
#include "../headers/tuple_space_linked_list.h"
#include <string.h>
#define PAYLOAD_NONCHARS_SIZE 36

int fieldcmp( field_t template, field_t field_2 )
{
    if ( template.type == field_2.type )
    {

        if ( template.is_actual == TS_NO )
        {

            return TS_SUCCESS;

        }
        else
        {

            if ( template.type == TS_INT )
            {
                if ( template.data.int_field == field_2.data.int_field )
                {

                    return TS_SUCCESS;

                }
                else
                {

                    return TS_FAILURE;

                }
            }
            if ( template.type == TS_FLOAT )
            {
                if ( template.data.float_field == field_2.data.float_field )
                {

                    return TS_SUCCESS;

                }
                else
                {

                    return TS_FAILURE;

                }
            }
        }
    }
    else
    {

        return TS_FAILURE;

    }
}

int idcmp( field_t id_1, field_t id_2 )
{

    if ( id_1.is_actual && id_2.is_actual )
    {

        if ( id_1.type == TS_INT && id_2.type == TS_INT )
        {

            if ( id_1.data.int_field == id_2.data.int_field )
            {

                return TS_SUCCESS;

            }
        }
        else
        {

            printf( "SERVER_ERROR: Tuple field id is not integer\n" );
            return TS_FAILURE;

        }
    }
    else
    {

        printf( "SERVER_ERROR: tuple's id field is not actual\n" );
        return TS_FAILURE;

    }
}

void char_to_tuple( char *str, tuple_t *tuple )
{

    int table[PAYLOAD_NONCHARS_SIZE / 4];
    memcpy( (void *)tuple, (void *)str, sizeof(tuple_t) ); //Copies whole buffer to tuple to fill out name field


    for ( int i = 0; i < PAYLOAD_NONCHARS_SIZE / 4; i++ )
    {
        //For every int in message copies it to proper address in integer table to "convert" chars to ints
        memcpy( (void *)&table[i], (void *)(str + sizeof(char) * 16 + i * sizeof(int)), sizeof(int) ); 

    }

    //Copies ints from integer table directly to tuple byte by byte for PAYLOAD_NONCHARS_SIZE
    memcpy( (void *)&tuple->tuple_len, (void *)table, PAYLOAD_NONCHARS_SIZE );

}

void tuple_to_char( tuple_t *tuple, char *str )
{

    memcpy( (void *)str, (void *)tuple, sizeof(tuple_t) );

}

void ts_out( tuple_t tuple )
{

    add_to_space( tuple ); 

}

void ts_inp( tuple_t template, tuple_t *retrive_tuple, int *inp_result )
{

    remove_from_space( template, retrive_tuple, inp_result ); 

}

void ts_rdp( tuple_t template, tuple_t *retrive_tuple, int *rdp_result )
{

    retrive_from_space( template, retrive_tuple, rdp_result ); 

}
