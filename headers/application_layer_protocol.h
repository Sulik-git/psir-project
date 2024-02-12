#ifndef APPLICATION_LAYER_PROTOCOL_H
#define APPLICATION_LAYER_PROTOCOL_H
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "../headers/alp_error.h"

#define PAYLOAD_SIZE 52
#define ALP_MESSAGE_MAXSIZE 54
#define ALP_ACKNOWLEDGED 1
#define ALP_NOT_ACKNOWLEDGED 0
#define ALP_BIN_ACK_FLAG 0b1
#define ALP_BIN_OP_YES_FLAG 0b1
#define ALP_BIN_OP_NO_FLAG 0b0
#define ALP_BIN_ACK_PAYLOAD 0b00
#define ALP_BIN_OUT_PAYLOAD 0b01
#define ALP_BIN_RDP_PAYLOAD 0b10
#define ALP_BIN_INP_PAYLOAD 0b11
#define ALP_ACK_OPERATION 0
#define ALP_OUT_OPERATION 1
#define ALP_RDP_OPERATION 2
#define ALP_INP_OPERATION 3

#define TIMEOUT_SEC 1
#define TIMEOUT_USEC 0

extern int alp_error;

struct alp_header
{

    uint8_t payload_type : 2;
    uint16_t sequence_number : 12;
    uint8_t op_result : 1;
    uint8_t acknowledge : 1;

};

struct alp_payload
{

    uint8_t block : 8;

};

struct alp_message
{

    struct alp_header header;
    struct alp_payload payload[PAYLOAD_SIZE];

};

int alp_init( char *port );
int alp_send( char *message, int operation, int op_result );
int alp_recv( char *recv_message, int *operation );
int alp_exit();
#endif