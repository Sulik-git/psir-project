#include "../headers/tuple_space_API.h"
#include "../headers/application_layer_protocol.h"
#include "../headers/tuple_space_linked_list.h"
#include "../headers/timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT "12345"

int main(void)
{
    int operation = 0;
    int op_result = 0;
    char character_buffor[PAYLOAD_SIZE];
    tuple_t tuple_buffor;
    tuple_t *tuple_buffor_p = &tuple_buffor;
    tuple_t retrive_tuple;
    tuple_t *retrive_tuple_p = &retrive_tuple;


    memset( character_buffor, '\0', PAYLOAD_SIZE );
    memset( tuple_buffor_p, 0, sizeof(tuple_t) );
    memset( retrive_tuple_p, 0, sizeof(tuple_t) );


    alp_init( PORT );
    printf( "SERVER_ON\n\n" );


    while (1)
    {

        alp_recv( character_buffor, &operation );
        if ( operation == TS_OUT )
        {

            char_to_tuple( character_buffor, tuple_buffor_p );
            ts_out( tuple_buffor );
            printnl_with_time( "SERVER: OUT operation complete - going back to Idle\n" );

        }
        else if ( operation == TS_RDP )
        {

            char_to_tuple( character_buffor, tuple_buffor_p );
            ts_rdp( tuple_buffor, retrive_tuple_p, &op_result );
            tuple_to_char( retrive_tuple_p, character_buffor );
            alp_send( character_buffor, TS_RDP, op_result );
            printnl_with_time( "SERVER: RDP operation complete - going back to Idle\n" );

        }
        else if ( operation == TS_INP )
        {

            char_to_tuple( character_buffor, tuple_buffor_p );
            ts_inp( tuple_buffor, retrive_tuple_p, &op_result );
            tuple_to_char( retrive_tuple_p, character_buffor );
            alp_send( character_buffor, TS_INP, op_result );
            printnl_with_time( "SERVER: INP operation complete - going back to Idle\n" );

        }

        op_result = 0;
        operation = 0;
        memset( character_buffor, '\0', PAYLOAD_SIZE );
        memset( tuple_buffor_p, 0, sizeof(tuple_t) );
        memset( retrive_tuple_p, 0, sizeof(tuple_t) );

    }

    alp_exit();
    return 0;
}
