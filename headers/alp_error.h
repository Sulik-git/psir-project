#ifndef ALP_ERROR_H
#define ALP_ERROR_H
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#define ALP_SUCCESS 0
#define ALP_FAILED -1
#define GETTADDRINFO_ERROR -14
#define SOCKET_ERROR -15
#define BIND_ERROR -16
#define SENDTO_ERROR -17
#define RECVROM_ERROR -18
#define SEND_ROUTINE_INIT_SOCKET_ERROR -26
#define SEND_ROUTINE_GETADDRINFO_ERROR -27
#define SEND_ROUTINE_SELECT_ERROR -19
#define SEND_ROUTINE_SENDFIRSTMSG_ERROR -21
#define SEND_ROUTINE_RECVFROM_ERROR -22
#define SEND_ROUTINE_RESEND_ERROR -23
#define RECV_ROUTINE_SELECT_ERROR -24
#define RECV_ROUTINE_INIT_SOCKET_ERROR -25
#define RECV_ROUTINE_RECVFROM_ERROR -28
#define RECV_ROUTINE_SENDTO_ERROR -29
#define SOCKET_ALERADY_EXISTS_ERROR -30
#define ADDRINFO_SOCKET_CREATION_ERROR -31

extern int errno;


int check_error( int error ); // Checks if function returns error code 

#endif