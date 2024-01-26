#include "../headers/alp_error.h"

#define GETTADDRINFO_ERROR_MSG "ALP_ERROR: Failed to get addrinfo for address\n"
#define SOCKET_ERROR_MSG "ALP_ERROR: Failed to create socket\n"
#define BIND_ERROR_MSG "ALP_ERROR: Failed to bind socket\n"
#define SENDTO_ERROR_MSG "ALP_ERROR: Failed to send ALP message\n"
#define RECVROM_ERROR_MSG "ALP_ERROR: Failed to recv ALP message\n"
#define SEND_ROUTINE_SELECT_ERROR_MSG "ALP_ERROR: Send Routine - Select() failed\n"
#define RECV_ROUTINE_SELECT_ERROR_MSG "ALP_ERROR: Recv Routine - Select() failed\n"
#define RECV_ROUTINE_RECVFROM_ERROR_MSG "ALP_ERROR: Recv Routine - recvfrom() failed\n"
#define RECV_ROUTINE_INIT_SOCKET_ERROR_MSG "ALP_ERROR: Recv Routine - Socket initialization failed\n "
#define RECV_ROUTINE_SENDTO_ERROR_MSG "ALP_ERROR: Recv Routine - Sending ack failed\n"
#define SEND_ROUTINE_GETADDRINFO_ERROR_MSG "ALP_ERROR: Send Routine - Getting addrinfo failed\n"
#define SEND_ROUTINE_RECVFROM_ERROR_MSG "ALP_ERROR: Send Routine - Recvfrom failed\n"
#define SEND_ROUTINE_INIT_SOCKET_ERROR_MSG "ALP_ERROR: Send Routine - Socket initiation failed\n"
#define SEND_ROUTINE_SENDFIRSTMSG_ERROR_MSG "ALP_ERROR: Send Routine - Sending first message failed\n"
#define SEND_ROUTINE_RESEND_ERROR_MSG "ALP_ERROR: Send Routine - Resending failed\n"
#define SOCKET_ALERADY_EXISTS_ERROR_MSG "ALP_ERROR: Socket already exists\n"
#define ADDRINFO_SOCKET_CREATION_ERROR_MSG "ALP_ERROR: Addrinfo failed during creating socket\n"
int alp_error = 0;
int errno_save = 0;

int check_error(int error)
{
    if (error < 0)
    {
        switch (error)
        {
        case GETTADDRINFO_ERROR:
            printf(GETTADDRINFO_ERROR_MSG);
            break;
        case SOCKET_ERROR:
            printf(SOCKET_ERROR_MSG);
            break;

        case BIND_ERROR:
            printf(BIND_ERROR_MSG);
            break;

        case SENDTO_ERROR:
            printf(SENDTO_ERROR_MSG);
            break;

        case RECVROM_ERROR:
            printf(RECVROM_ERROR_MSG);
            break;

        case SEND_ROUTINE_SELECT_ERROR:
            printf(SEND_ROUTINE_SELECT_ERROR_MSG);
            break;

        case RECV_ROUTINE_SELECT_ERROR:
            printf(RECV_ROUTINE_SELECT_ERROR_MSG);
            break;

        case RECV_ROUTINE_RECVFROM_ERROR:
            printf(RECV_ROUTINE_RECVFROM_ERROR_MSG);
            break;

        case RECV_ROUTINE_INIT_SOCKET_ERROR:
            printf(RECV_ROUTINE_INIT_SOCKET_ERROR_MSG);
            break;

        case SEND_ROUTINE_GETADDRINFO_ERROR:
            printf(SEND_ROUTINE_GETADDRINFO_ERROR_MSG);
            break;

        case SEND_ROUTINE_RECVFROM_ERROR:
            printf(SEND_ROUTINE_RECVFROM_ERROR_MSG);
            break;

        case SEND_ROUTINE_INIT_SOCKET_ERROR:
            printf(SEND_ROUTINE_INIT_SOCKET_ERROR_MSG);
            break;

        case SEND_ROUTINE_SENDFIRSTMSG_ERROR:
            printf(SEND_ROUTINE_SENDFIRSTMSG_ERROR_MSG);
            break;

        case SEND_ROUTINE_RESEND_ERROR:
            printf(SEND_ROUTINE_RESEND_ERROR_MSG);
            break;

        case RECV_ROUTINE_SENDTO_ERROR:
            printf(RECV_ROUTINE_SENDTO_ERROR_MSG);
            break;

        case SOCKET_ALERADY_EXISTS_ERROR:
            printf(SOCKET_ALERADY_EXISTS_ERROR_MSG);
            break;

        default:
            break;
        }
        fprintf(stderr, "ERRORNUM: %s\n", strerror(errno_save));
        return ALP_FAILED;
    }
    return ALP_SUCCESS;
}