#include "../headers/application_layer_protocol.h"

extern int alp_error;
extern int errno_save;
static int is_socket_created = 0;
static int sockfd = 0;
static struct sockaddr other_host;

/* TODO:
        - Need to modify protocol
        - Add char* to struct alp_message translation to properly abstract
        - Char to alp_message convertion

            Opt:
            - Not sure how full bit field alp message will work, HAVE IN MIND
            - Add special functions for preparing diffrent type of msgs
            - Repair message size 132 -> 130. why 132?
            - Better ack msg checking using
            - Add "Lost Connection" timeout
 */

// Getting address info
int alp_getaddrinfo(char *ip, char *port, struct addrinfo *alp_addrinfo)
{
    struct addrinfo h, r, *a = &r; // Segfault when passing address of pointer of struct cause uninitialized???
    memset(&h, 0, sizeof(struct addrinfo));
    h.ai_family = PF_INET;
    h.ai_socktype = SOCK_DGRAM;
    h.ai_flags = AI_PASSIVE;
    alp_error = getaddrinfo(ip, port, &h, &a);
    if (alp_error != 0)
    {
        errno_save = errno;
        return GETTADDRINFO_ERROR;
    }
    else
    {
        memcpy(alp_addrinfo, a, sizeof(struct addrinfo));
        alp_addrinfo->ai_next = NULL;
        freeaddrinfo(a);
        return ALP_SUCCESS;
    }
}

// Creating socket
int alp_create_sockfd(char *port)
{
    if (is_socket_created == 0)
    {
        struct addrinfo c, *d = NULL;
        d = &c;
        alp_error = alp_getaddrinfo(NULL, port, d);
        if (check_error(alp_error))
        {
            return ADDRINFO_SOCKET_CREATION_ERROR;
        }

        sockfd = socket(c.ai_family, c.ai_socktype, c.ai_protocol);
        if (sockfd == -1)
        {
            errno_save = errno;
            return SOCKET_ERROR;
        }

        alp_error = bind(sockfd, c.ai_addr, c.ai_addrlen);
        if (alp_error == -1)
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

int check_operation(struct alp_message message)
{
    if (message.header.payload_type == ALP_BIN_OUT_PAYLOAD)
    {
        return ALP_OUT_OPERATION;
    }
    if (message.header.payload_type == ALP_BIN_RDP_PAYLOAD)
    {
        return ALP_RDP_OPERATION;
    }
    if (message.header.payload_type == ALP_BIN_INP_PAYLOAD)
    {
        return ALP_INP_OPERATION;
    }
    return 0;
}

int prepare_alp_message(char *message, struct alp_message *msg_to_prepare, int operation, int rdp_flag)
{
    if (operation == ALP_ACK_OPERATION)
    {
        memset(msg_to_prepare, 0, sizeof(struct alp_message));
        msg_to_prepare->header.acknowledge = ALP_BIN_ACK_FLAG;
        msg_to_prepare->header.payload_type = ALP_BIN_ACK_PAYLOAD;
        msg_to_prepare->header.sequence_number = 0b0;
    }
    if (operation == ALP_RDP_OPERATION)
    {
        memset(msg_to_prepare, 0, sizeof(struct alp_message));
        msg_to_prepare->header.acknowledge = ALP_BIN_RDP_NO_FLAG;
        msg_to_prepare->header.payload_type = ALP_BIN_RDP_PAYLOAD;
        if (rdp_flag)
        {
            msg_to_prepare->header.rdp_result = ALP_BIN_RDP_YES_FLAG;
        }
        else
        {
            msg_to_prepare->header.rdp_result = ALP_BIN_RDP_NO_FLAG;
        }
        msg_to_prepare->header.sequence_number = 0b0;
        memcpy((void *)&msg_to_prepare->payload, (void *)message, PAYLOAD_SIZE);
    }
    return ALP_SUCCESS;
}

// Alp sendto() implemntation
int alp_sendto(struct alp_message *message, int alp_message_len, struct sockaddr *a_in, int a_in_len)
{
    int sent = 0;
    char msg[ALP_MESSAGE_MAXSIZE];
    memcpy((void *)msg, (void *)message, ALP_MESSAGE_MAXSIZE);
    sent = sendto(sockfd, msg, alp_message_len, 0, a_in, a_in_len);
    if (sent <= 0)
    {
        errno_save = errno;
        return SENDTO_ERROR;
    }
    else
    {
        printf("\nALP: Message of %d bytes sent\n\n", sent);
        return sent;
    }
}

// Alp recvfrom() implemntation
int alp_recvfrom(char *buffer, int buffer_len, struct sockaddr *a_in, int a_in_len)
{
    memset(buffer, 0, buffer_len);
    int pos = 0;
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

int alp_send_routine(char *message, int operation, int rdp_result)
{
    // Initializations
    int ack = 0;
    struct sockaddr saved_other_host;
    struct alp_message send_message;
    struct alp_message inc_message;
    char buff[ALP_MESSAGE_MAXSIZE];
    int x = 0;
    fd_set readfds, readyfds;
    struct timeval stv, ttv, dttv, sttv;
    stv.tv_sec = 0;
    stv.tv_usec = 0;
    ttv.tv_sec = TIMEOUT_SEC;
    ttv.tv_usec = TIMEOUT_USEC;

    // Preparing sets for select()
    memset(&inc_message, 0, sizeof(struct alp_message));
    memset(&send_message, 0, sizeof(struct alp_message));
    memset(&saved_other_host, 0, sizeof(struct sockaddr));

    prepare_alp_message(message, &send_message, operation, rdp_result);
    ;
    // Sending first message and starting timeout clock
    alp_error = alp_sendto(&send_message, ALP_MESSAGE_MAXSIZE, &other_host, sizeof(other_host));
    if (check_error(alp_error))
    {
        return SEND_ROUTINE_SENDFIRSTMSG_ERROR;
    }
    saved_other_host = other_host;
    gettimeofday(&sttv, NULL);

    // Waiting for ACK
    while (ack == ALP_NOT_ACKNOWLEDGED)
    {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        readyfds = readfds;
        x = select(1024, &readyfds, NULL, NULL, &stv);
        if (x < 0)
        {
            errno_save = errno;
            return SEND_ROUTINE_SELECT_ERROR;
        }
        else if (x > 0)
        {
            if (FD_ISSET(sockfd, &readyfds))
            {
                alp_error = alp_recvfrom(buff, ALP_MESSAGE_MAXSIZE, &other_host, sizeof(other_host));
                if (check_error(alp_error))
                {
                    return SEND_ROUTINE_RECVFROM_ERROR;
                }
                memcpy((void *)&inc_message, (void *)buff, ALP_MESSAGE_MAXSIZE);
                if ((strcmp(saved_other_host.sa_data, other_host.sa_data)) == 0 && inc_message.header.acknowledge & ALP_BIN_ACK_FLAG) // Checking if address is the same as the one that was object of sending and checking for Ack flag
                {
                    ack = ALP_ACKNOWLEDGED;
                    continue;
                }
            }
        }

        // Checking for timeout
        gettimeofday(&dttv, NULL);
        if (((dttv.tv_sec - sttv.tv_sec) >= ttv.tv_sec) && ((dttv.tv_usec - sttv.tv_usec) >= ttv.tv_usec))
        {
            alp_error = alp_sendto(&send_message, ALP_MESSAGE_MAXSIZE, &saved_other_host, sizeof(saved_other_host));
            if (alp_error == SENDTO_ERROR) // Resending message in case of timeout
            {
                return SEND_ROUTINE_RESEND_ERROR;
            }
            gettimeofday(&sttv, NULL); // Reseting timer
        }
    }
    return ALP_SUCCESS;
}

int alp_recv_routine(char *recv_message)
{
    // Initializations
    char test[PAYLOAD_SIZE];
    int x = 0;
    struct timeval stv;
    stv.tv_sec = 1;
    stv.tv_usec = 0;
    struct alp_message ack_message, recv_alp_message;
    static char buff[ALP_MESSAGE_MAXSIZE];
    fd_set readfds, readyfds;

    // Preparing ack message
    prepare_alp_message(NULL, &ack_message, ALP_ACK_OPERATION, 0);

    // Preperaing sets for select()
    memset(&other_host, 0, sizeof(struct sockaddr));
    memset(&recv_alp_message, 0, sizeof(struct alp_message));
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    readyfds = readfds;
    x = select(1024, &readyfds, NULL, NULL, &stv);
    if (x < 0)
    {
        errno_save = errno;
        return RECV_ROUTINE_SELECT_ERROR;
    }
    else if (x > 0)
    {
        if (FD_ISSET(sockfd, &readyfds))
        {
            alp_error = alp_recvfrom(buff, ALP_MESSAGE_MAXSIZE, &other_host, sizeof(struct sockaddr));
            if (check_error(alp_error)) // If select() is ready reciving message and sending ack message
            {
                return RECV_ROUTINE_RECVFROM_ERROR;
            }

            alp_error = alp_sendto(&ack_message, ALP_MESSAGE_MAXSIZE, &other_host, sizeof(struct sockaddr));
            if (check_error(alp_error))
            {
                return RECV_ROUTINE_SENDTO_ERROR;
            }
            // Converting chars for alp_message and memcpying to provided address
            memcpy((void *)&recv_alp_message, (void *)buff, ALP_MESSAGE_MAXSIZE);
            memcpy((void *)recv_message, (void *)&recv_alp_message.payload, PAYLOAD_SIZE);
        }
    }
    return check_operation(recv_alp_message);
}

int alp_init(char *port)
{
    if (check_error(alp_create_sockfd(port)))
        return ALP_FAILED;

    return ALP_SUCCESS;
}
int alp_send(char *message, int operation, int rdp_result)
{
    if (check_error(alp_send_routine(message, operation, rdp_result)))
        return ALP_FAILED;

    return ALP_SUCCESS;
}
int alp_recv(char *recv_message, int *recv_operation)
{
    alp_error = alp_recv_routine(recv_message);
    if (check_error(alp_error))
    {
        recv_operation = 0;
        return ALP_FAILED;
    }
    *recv_operation = alp_error;
    return ALP_SUCCESS;
}
int alp_exit()
{
    close(sockfd);
    return ALP_SUCCESS;
}