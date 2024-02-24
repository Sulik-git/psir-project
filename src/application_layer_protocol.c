#include "../headers/application_layer_protocol.h"
#include "../headers/print_with_time.h"

extern int alp_error;
extern int errno_save;
static int is_socket_created = 0;
static int sockfd = 0;
static struct sockaddr other_host;


/* TODO:
    - Add hton and ntoh in alp_header
    - Add message seriaization for instance MessagePack
    - Add dynamic alp_messages (if message is ack send only header etc.)
*/



// Getting address info
int alp_getaddrinfo( char *ip, char *port, struct addrinfo *alp_addrinfo )
{

    struct addrinfo h, r, *a = &r; 

    
    memset( &h, 0, sizeof(struct addrinfo) );
    h.ai_family = PF_INET;
    h.ai_socktype = SOCK_DGRAM;
    h.ai_flags = AI_PASSIVE;


    alp_error = getaddrinfo( ip, port, &h, &a );
    if ( alp_error != 0 )
    {

        errno_save = errno;
        return GETTADDRINFO_ERROR;

    }
    else
    {

        memcpy( alp_addrinfo, a, sizeof(struct addrinfo) );
        alp_addrinfo->ai_next = NULL;
        freeaddrinfo( a );
        return ALP_SUCCESS;

    }
}

// Creating socket
int alp_create_sockfd( char *port )
{

    if ( is_socket_created == 0 )
    {

        struct addrinfo c, *d = NULL;
        d = &c;
        

        alp_error = alp_getaddrinfo( NULL, port, d );
        if ( check_error(alp_error) )
        {
            return ADDRINFO_SOCKET_CREATION_ERROR;
        }


        sockfd = socket( c.ai_family, c.ai_socktype, c.ai_protocol );
        if ( sockfd == -1 )
        {
            errno_save = errno;
            return SOCKET_ERROR;
        }


        alp_error = bind( sockfd, c.ai_addr, c.ai_addrlen );
        if ( alp_error == -1 )
        {
            errno_save = errno;
            return BIND_ERROR;
        }


        is_socket_created = 1;
        return ALP_SUCCESS;

    }
    else
    {

        return SOCKET_ALERADY_EXISTS_ERROR;

    }
}

int check_operation( struct alp_message message )
{

    if ( message.header.payload_type == ALP_BIN_OUT_PAYLOAD )
    {

        return ALP_OUT_OPERATION;

    }
    if ( message.header.payload_type == ALP_BIN_RDP_PAYLOAD )
    {

        return ALP_RDP_OPERATION;

    }
    if ( message.header.payload_type == ALP_BIN_INP_PAYLOAD )
    {

        return ALP_INP_OPERATION;

    }


    return 0;

}


void alp_header_hton( struct alp_message *temp_message, struct alp_message_n *network_message ) // Changes endiannes from host to network in header
{

    memcpy( (void *)&network_message->header, (void *)&temp_message->header, sizeof(uint16_t) );
    

    network_message->header = htons(network_message->header);

}


void alp_header_ntoh( struct alp_message_n *received_network_message, struct alp_message *received_message ) // Changes endiannes from network to host in header
{

    received_network_message->header = ntohs(received_network_message->header);
    
    memcpy( (void *)&received_message->header, (void *)&received_network_message->header, sizeof(uint16_t) );

}


int prepare_alp_message( char *message, struct alp_message_n *msg_to_prepare, int operation, int op_flag ) // Perpares alp_message by setting header flags with proper values and copies payload to alp_message structure 
{

    struct alp_message temp_alp_buffor;


    // Zeroing structs
    memset( (void *)&temp_alp_buffor, 0, sizeof(struct alp_message));
    memset( (void *)msg_to_prepare, 0, sizeof(struct alp_message_n) );


    // Preparing message accordingly to selected operation
    if ( operation == ALP_ACK_OPERATION )
    {
         
        temp_alp_buffor.header.acknowledge = ALP_BIN_ACK_FLAG;
        temp_alp_buffor.header.payload_type = ALP_BIN_ACK_PAYLOAD;
        temp_alp_buffor.header.sequence_number = 0b0;


        alp_header_hton( &temp_alp_buffor, msg_to_prepare );

    }
    else if ( operation == ALP_RDP_OPERATION )
    {

        temp_alp_buffor.header.payload_type = ALP_BIN_RDP_PAYLOAD;
        temp_alp_buffor.header.sequence_number = 0b0;


        if ( op_flag )
        {
            temp_alp_buffor.header.op_result = ALP_BIN_OP_YES_FLAG;
        }
        else
        {
            temp_alp_buffor.header.op_result = ALP_BIN_OP_NO_FLAG;
        }


        alp_header_hton( &temp_alp_buffor, msg_to_prepare );
        memcpy( (void *)&msg_to_prepare->payload, (void *)message, PAYLOAD_SIZE );

    }
    else if ( operation == ALP_INP_OPERATION )
    {

        temp_alp_buffor.header.payload_type = ALP_BIN_INP_PAYLOAD;
        temp_alp_buffor.header.sequence_number = 0b0;
        
        if ( op_flag )
        {
            temp_alp_buffor.header.op_result = ALP_BIN_OP_YES_FLAG;
        }
        else
        {
            temp_alp_buffor.header.op_result = ALP_BIN_OP_NO_FLAG;
        }


        alp_header_hton( &temp_alp_buffor, msg_to_prepare );
        memcpy( (void *)&msg_to_prepare->payload, (void *)message, PAYLOAD_SIZE );

    }


    return ALP_SUCCESS;

}

// Alp sendto() implemntation
int alp_sendto( struct alp_message_n *message, int alp_message_len, struct sockaddr *a_in, int a_in_len )
{
    int sent = 0;
    char msg[ALP_MESSAGE_MAXSIZE];


    memcpy( (void *)msg, (void *)message, ALP_MESSAGE_MAXSIZE );


    sent = sendto( sockfd, msg, alp_message_len, 0, a_in, a_in_len );
    if ( sent <= 0 )
    {

        errno_save = errno;
        return SENDTO_ERROR;

    }
    else
    {

        //printf( "ALP: Message of %d bytes sent\n", sent );
        return sent;

    }

}

// Alp recvfrom() implemntation
int alp_recvfrom( char *buffer, int buffer_len, struct sockaddr *a_in, int a_in_len )
{

    int pos = 0;


    memset( buffer, 0, buffer_len );


    if (pos = recvfrom(sockfd, buffer, buffer_len, 0, a_in, &a_in_len) < 0)
    {

        errno_save = errno;
        return RECVROM_ERROR;

    }
    else
    {

        return pos;

    }

}

int alp_send_routine( char *message, int operation, int op_result )
{

    // Initializations
    struct sockaddr saved_other_host;
    struct alp_message_n send_network_message;
    struct alp_message_n inc_network_message;
    struct alp_message inc_message;
    char buff[ALP_MESSAGE_MAXSIZE];
    int resend_counter = 0;
    int x = 0;
    fd_set readfds, readyfds;
    struct timeval stv, ttv, dttv, sttv;
    stv.tv_sec = 0;
    stv.tv_usec = 0;
    ttv.tv_sec = TIMEOUT_SEC;
    ttv.tv_usec = TIMEOUT_USEC;


    // Preparing sets for select()
    memset( (void *)&inc_network_message, 0, sizeof(struct alp_message_n) );
    memset( (void *)&send_network_message, 0, sizeof(struct alp_message_n) );
    memset( (void *)&inc_message, 0, sizeof(struct alp_message) );
    memset( (void *)&saved_other_host, 0, sizeof(struct sockaddr) );


    prepare_alp_message( message, &send_network_message, operation, op_result );

    // Sending first message and starting timeout clock
    alp_error = alp_sendto( &send_network_message, ALP_MESSAGE_MAXSIZE, &other_host, sizeof(other_host) );
    if ( check_error(alp_error) )
    {

        return SEND_ROUTINE_SENDFIRSTMSG_ERROR;

    }
    printnl_with_time( "ALP: Payload message sent" );
    saved_other_host = other_host;
    gettimeofday( &sttv, NULL );


    // Waiting for ACK
    while ( 1 )
    {

        FD_ZERO( &readfds );
        FD_SET( sockfd, &readfds );
        readyfds = readfds;


        x = select( 1024, &readyfds, NULL, NULL, &stv );
        if ( x < 0 )
        {

            errno_save = errno;
            return SEND_ROUTINE_SELECT_ERROR;

        }
        else if ( x > 0 )
        {

            if ( FD_ISSET(sockfd, &readyfds) )
            {

                alp_error = alp_recvfrom( buff, ALP_MESSAGE_MAXSIZE, &other_host, sizeof(other_host) );
                if ( check_error(alp_error) )
                {

                    return SEND_ROUTINE_RECVFROM_ERROR;

                }


                memcpy( (void *)&inc_network_message, (void *)buff, ALP_MESSAGE_MAXSIZE );

                alp_header_ntoh( &inc_network_message, &inc_message );

                if ( (strcmp(saved_other_host.sa_data, other_host.sa_data)) == 0 && inc_message.header.acknowledge & ALP_BIN_ACK_FLAG ) // Checking if address is the same as the one that was object of sending and checking for Ack flag
                {

                    printnl_with_time( "ALP: Recived ACK message" );
                    break;

                }
            }
        }

        // Checking for timeout
        gettimeofday( &dttv, NULL );
        if ( ((dttv.tv_sec - sttv.tv_sec) >= ttv.tv_sec) && ((dttv.tv_usec - sttv.tv_usec) >= ttv.tv_usec) )
        {

            if ( resend_counter < 6 )
            {

                alp_error = alp_sendto( &send_network_message, ALP_MESSAGE_MAXSIZE, &saved_other_host, sizeof(saved_other_host) );
                if ( alp_error == SENDTO_ERROR ) // Resending message in case of timeout
                {
                    return SEND_ROUTINE_RESEND_ERROR;
                }


                printnl_with_time( "ALP: Message resent" );


                gettimeofday( &sttv, NULL ); // Reseting timer


                resend_counter++;

            }
            else
            {

                printnl_with_time( "ALP: Message resending timed out" );
                printnl_with_time( "ALP: Server going back to Idle" );
                break;

            }
        }
    }


    return ALP_SUCCESS;

}

int alp_recv_routine( char *recv_message )
{
    // Initializations
    char test[PAYLOAD_SIZE];
    int x = 0;
    struct timeval stv;
    stv.tv_sec = 1;
    stv.tv_usec = 0;
    struct alp_message_n ack_network_message, recv_network_message;
    struct alp_message recv_alp_message;
    static char buff[ALP_MESSAGE_MAXSIZE];
    fd_set readfds, readyfds;


    // Preparing ack message
    prepare_alp_message( NULL, &ack_network_message, ALP_ACK_OPERATION, 0 );


    // Preperaing sets for select()
    memset( (void *)&other_host, 0, sizeof(struct sockaddr) );
    memset( (void *)&recv_network_message, 0, sizeof(struct alp_message_n) );
    memset( (void *)&recv_alp_message, 0, sizeof(struct alp_message) );
    FD_ZERO( &readfds );
    FD_SET( sockfd, &readfds );


    readyfds = readfds;
    x = select( 1024, &readyfds, NULL, NULL, &stv );
    if ( x < 0 )
    {

        errno_save = errno;
        return RECV_ROUTINE_SELECT_ERROR;

    }
    else if ( x > 0 )
    {

        if ( FD_ISSET( sockfd, &readyfds) )
        {

            alp_error = alp_recvfrom( buff, ALP_MESSAGE_MAXSIZE, &other_host, sizeof(struct sockaddr) );
            if ( check_error(alp_error) ) // If select() is ready reciving message and sending ack message
            {

                return RECV_ROUTINE_RECVFROM_ERROR;

            }
            

            printnl_with_time( "ALP: Received Payload message" );


            alp_error = alp_sendto( &ack_network_message, ALP_MESSAGE_MAXSIZE, &other_host, sizeof(struct sockaddr) );
            if ( check_error(alp_error) )
            {

                return RECV_ROUTINE_SENDTO_ERROR;

            }


            printnl_with_time( "ALP: ACK message sent" );


            // Converting chars for alp_message and memcpying to provided address
            memcpy( (void *)&recv_network_message, (void *)buff, ALP_MESSAGE_MAXSIZE ); 
            alp_header_ntoh( &recv_network_message, &recv_alp_message );
            memcpy( (void *)recv_message, (void *)&recv_network_message.payload, PAYLOAD_SIZE );

        }
    }


    return check_operation(recv_alp_message);

}

int alp_init( char *port )
{

    if ( check_error(alp_create_sockfd(port)) )
        return ALP_FAILED;


    return ALP_SUCCESS;

}
int alp_send( char *message, int operation, int rdp_result )
{
    if ( check_error(alp_send_routine(message, operation, rdp_result)) )
        return ALP_FAILED;


    return ALP_SUCCESS;

}
int alp_recv( char *recv_message, int *recv_operation )
{

    alp_error = alp_recv_routine( recv_message );
    if ( check_error(alp_error) )
    {
        recv_operation = 0;
        return ALP_FAILED;
    }
    *recv_operation = alp_error;


    return ALP_SUCCESS;

}
int alp_exit()
{

    close( sockfd );
    return ALP_SUCCESS;

}