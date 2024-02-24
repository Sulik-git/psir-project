#include <stdio.h>
#include "../headers/tuple_space_API.h"
#include "../headers/tuple_space_linked_list.h"
#include <arpa/inet.h>
#include <string.h>
#define PAYLOAD_NONCHARS_SIZE 36


void field_hton( field_n_t *field )                 // Changes endianess from host to network ( Little -> Big ) for whole tuple's field
{

    field->is_actual = htonl( field->is_actual );
    field->type = htonl( field->type );


    // Does htonl operation only for one type
    if( field->type == TS_INT )
    {

        field->data.int_field = htonl( field->data.int_field );

    }
    else if( field->type == TS_FLOAT )
    {

        field->data.float_field = htonl( field->data.float_field );

    }

}


void field_ntoh( field_n_t *field)                  // Changes endianess from network to host ( Big -> Little ) for whole tuple's field
{

    field->is_actual = ntohl( field->is_actual );
    field->type = ntohl( field->type );


    // Does htonl operation only for one type
    if( field->type == TS_INT )
    {

        field->data.int_field = ntohl ( field->data.int_field );

    }
    else if( field->type == TS_FLOAT )
    {
    
        field->data.float_field = ntohl ( field->data.float_field );

    }

}
        
int fieldcmp( field_t template, field_t field_2 )       // Comparing two fields ( template's field to tuple's field)
{
    if ( template.type == field_2.type )        // Checks type
    {

        if ( template.is_actual == TS_NO )      // Checks if field is not actual ( when template field is not actual then imidiately returns true )
        {

            return TS_SUCCESS;

        }
        else
        {

            if ( template.type == TS_INT )                                  // Checks for same data type
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

int idcmp( field_t id_1, field_t id_2 )             // Compares ids of two fields
{

    if ( id_1.is_actual && id_2.is_actual )         // Id must be actual
    {

        if ( id_1.type == TS_INT && id_2.type == TS_INT )   // Id must be INT
        {

            if ( id_1.data.int_field == id_2.data.int_field )  // Checking actual value
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

void char_to_tuple( char *str, tuple_t *tuple )                         // Changes character buffer to tuple and changes endianess of data
{

    tuple_n_t network_tuple;


    memcpy( (void *)&network_tuple, (void *)str, sizeof(tuple_n_t) );             // Copies whole buffer to tuple to fill out name field
    memcpy( (void *)&tuple->name, (void *)&network_tuple , NAME_MAX_SIZE );       // Copying name from network tuple to actual tuple 

    
    for( size_t s = 0; s < TUPLE_MAX_SIZE; s++)
    {
        
        field_ntoh( &network_tuple.field[s] );     // Ntoh whole field 


        // Assaigning value after value from network tuple to actual tuple; Only one data type is assigned
        tuple->tuple_fields[s].is_actual = (int32_t)network_tuple.field[s].is_actual;                    
        tuple->tuple_fields[s].type = (int32_t)network_tuple.field[s].type;
        if( network_tuple.field[s].type == TS_INT )
        {

            tuple->tuple_fields[s].data.int_field = (int32_t)network_tuple.field[s].data.int_field;

        }
        else if( network_tuple.field[s].type == TS_FLOAT )
        {

            tuple->tuple_fields[s].data.float_field = (float)network_tuple.field[s].data.float_field;

        }
        tuple->tuple_fields[s].padding = (int32_t)network_tuple.field[s].padding;

    }


    network_tuple.field_num = ntohl( network_tuple.field_num );  // Independently ntohl number of fields and then assigning it
    tuple->tuple_len = (int32_t)network_tuple.field_num;

}


void tuple_to_char( tuple_t *tuple, char *str )                 // Changes tuple to character buffer and changes endianess of data
{

    tuple_n_t network_tuple;


    //Copies values of each element of tuple to network tuple and then changes endianess from host to network
    memcpy( network_tuple.name, &tuple->name, NAME_MAX_SIZE );
    network_tuple.field_num = (uint32_t)tuple->tuple_len;
    for( size_t s = 0; s < TUPLE_MAX_SIZE; s++ )
    {

        network_tuple.field[s].is_actual = (uint32_t)tuple->tuple_fields[s].is_actual;
        network_tuple.field[s].type = (uint32_t)tuple->tuple_fields[s].type;
        if( network_tuple.field[s].type == TS_INT )
        {

            network_tuple.field[s].data.int_field = (uint32_t)tuple->tuple_fields[s].data.int_field;

        }
        else if( network_tuple.field[s].type == TS_FLOAT )
        {

            memcpy( &network_tuple.field[s].data.float_field, &tuple->tuple_fields->data.float_field, sizeof(float) );

        }
        network_tuple.field[s].padding = (uint32_t)tuple->tuple_fields[s].padding;


        field_hton( &network_tuple.field[s] );

    }
    network_tuple.field_num = htonl( network_tuple.field_num );


    //Copies bytes from network tuple to char buffer
    memcpy( (void *)str, (void *)&network_tuple, sizeof(tuple_n_t) );

}

void ts_out( tuple_t tuple )                // Adds tuple to tuple_space
{

    add_to_space( tuple ); 

}

void ts_inp( tuple_t template, tuple_t *retrive_tuple, int *inp_result )        // Retrives tuple form tuple field deleting it and sets operation result
{

    remove_from_space( template, retrive_tuple, inp_result ); 

}

void ts_rdp( tuple_t template, tuple_t *retrive_tuple, int *rdp_result )        // Retrives tuple form tuple field without deleting and sets operation result
{

    retrive_from_space( template, retrive_tuple, rdp_result ); 

}
