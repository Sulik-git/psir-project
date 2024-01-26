#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/application_layer_protocol.h"

#define SERVER_PORT "13456"
#define CLIENT_PORT "12456"
#define SERVER_IP "127.0.0.1"

int main(int argv, char *argc[])
{ /*
     if (argv < 2)
     {
         printf("To use test pls specify if process is client or server\n");
         exit(EXIT_FAILURE);
     }
     else
     {
         if (argc[1][0] == 's')
         {
             alp_init(SERVER_PORT);
             printf("SERVER MODE\nWaiting for message...\n");
             char *test = "test";
             char buffer[128];
             int operation = 0;
             while (strcmp(buffer, test))
             {
                 alp_recv(buffer, &operation);
             }
             printf("Message received\n");
             alp_exit();
         }
         else if (argc[1][0] == 'c')
         {
             alp_init(CLIENT_PORT);
             int error = 0;
             char buff[PAYLOAD_SIZE] = "test";
             printf("CLIENT MODE\n");
             alp_send(buff, ALP_OUT_OPERATION);
             printf("Routine completed\n");
         }
         else
         {
             printf("Use s for server or c client\n");
         }
     }
     return 0;
     */
}