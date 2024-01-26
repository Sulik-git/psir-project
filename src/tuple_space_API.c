#include "../headers/application_layer_protocol.h"
#include "../headers/tuple_space_API.h"
#include "../headers/tuple_space_linked_list.h"
#include <stdio.h>
#define PAYLOAD_NONCHARS_SIZE 132
#define TS_OUT 2
#define TS_INP 3
#define TS_RDP 4

int fieldcmp(field_t template, field_t field_2)
{
    if (template.type == field_2.type)
    {
        if (template.is_actual == TS_NO)
        {
            return TS_SUCCESS;
        }
        else
        {
            if (template.type == TS_INT)
            {
                if (template.data.int_field == field_2.data.int_field)
                {
                    return TS_SUCCESS;
                }
                else
                {
                    return TS_FAILURE;
                }
            }
            if (template.type == TS_FLOAT)
            {
                if (template.data.float_field == field_2.data.float_field)
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

int idcmp(field_t id_1, field_t id_2)
{
    if (id_1.is_actual && id_2.is_actual)
    {
        if (id_1.type == TS_INT && id_2.type == TS_INT)
        {
            if (id_1.data.int_field == id_2.data.int_field)
            {
                return TS_SUCCESS;
            }
        }
        else
        {
            printf("SERVER_ERROR: Tuple field id is not integer\n");
            return TS_FAILURE;
        }
    }
    else
    {
        printf("SERVER_ERROR: tuple's id field is not actual\n");
        return TS_FAILURE;
    }
}

void char_to_tuple(char *str, tuple_t *tuple)
{
    int table[PAYLOAD_NONCHARS_SIZE / 4];
    memcpy((void *)tuple, (void *)str, sizeof(tuple_t));
    for (int i = 0; i < 33; i++)
    {
        memcpy((void *)&table[i], (void *)(str + sizeof(char) * 16 + i * sizeof(int)), sizeof(int));
    }
    memcpy((void *)&tuple->tuple_len, (void *)table, PAYLOAD_NONCHARS_SIZE);
}

void tuple_to_char(tuple_t *tuple, char *str)
{
    memcpy((void *)str, (void *)tuple, sizeof(tuple_t));
}

void ts_out(tuple_t tuple)
{
    add_to_space(tuple); // Add error detection
}

void ts_inp(tuple_t template, tuple_t *retrive_tuple)
{
    remove_from_space(template, retrive_tuple); // Add error detection
}

void ts_rdp(tuple_t template, tuple_t *retrive_tuple, int *rdp_result)
{
    retrive_from_space(template, retrive_tuple, rdp_result); // Add error detection
}

void cycle(char *port)
{
    int operation = 0;
    int rdp_result = 0;
    char character_buffor[PAYLOAD_SIZE];
    tuple_t tuple_buffor;
    tuple_t *tuple_buffor_p = &tuple_buffor;
    tuple_t retrive_tuple;
    tuple_t *retrive_tuple_p = &retrive_tuple;
    memset(character_buffor, '\0', PAYLOAD_SIZE);
    memset(tuple_buffor_p, 0, sizeof(tuple_t));
    memset(retrive_tuple_p, 0, sizeof(tuple_t));
    alp_init(port);
    printf("SERVER_ON\n");
    while (1)
    {
        alp_recv(character_buffor, &operation);
        if (operation == TS_OUT)
        {
            char_to_tuple(character_buffor, tuple_buffor_p);
            ts_out(tuple_buffor);
        }
        else if (operation == TS_RDP)
        {
            char_to_tuple(character_buffor, tuple_buffor_p);
            ts_rdp(tuple_buffor, retrive_tuple_p, &rdp_result);
            tuple_to_char(retrive_tuple_p, character_buffor);
            alp_send(character_buffor, TS_RDP, rdp_result);
        }
        else if (operation == TS_INP)
        {
            char_to_tuple(character_buffor, tuple_buffor_p);
            ts_inp(tuple_buffor, retrive_tuple_p);
            tuple_to_char(retrive_tuple_p, character_buffor);
            alp_send(character_buffor, TS_INP, rdp_result);
        }
        rdp_result = 0;
        operation = 0;
        memset(character_buffor, '\0', PAYLOAD_SIZE);
        memset(tuple_buffor_p, 0, sizeof(tuple_t));
        memset(retrive_tuple_p, 0, sizeof(tuple_t));
    }

    alp_exit();
}