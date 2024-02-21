#ifndef TUPLE_SPACE_API_H
#define TUPLE_SPACE_API_H


#define TS_YES 1
#define TS_NO 0
#define TS_INT 0
#define TS_FLOAT 1
#define TS_SUCCESS 1
#define TS_FAILURE 0
#define TS_OUT 1
#define TS_RDP 2
#define TS_INP 3

#define NAME_MAX_SIZE 16
#define TUPLE_MAX_SIZE 2 

typedef struct
{

    int is_actual;      // 0 if field is not actual; 1 if field is actual
    int type;           // 0 if data is int; 1 if data is float;
    union
    {
        int int_field;
        float float_field;
    } data;                     // Union containing field's data
    int padding;                // Padding added so field is 16 Bytes

} field_t;  

typedef struct
{

    char name[NAME_MAX_SIZE];             // 16B   Tuple name
    int tuple_len;                        // 4B    Number of tuple's fields
    field_t tuple_fields[TUPLE_MAX_SIZE]; // 32B   Tuple's fields

} tuple_t; // 52B        Struct representation of tuple

int fieldcmp( field_t template, field_t field_2 );      // Compares two fields and returns 1 if both are the same ( also true in case of template's field being not actual )

int idcmp( field_t id_1, field_t id_2 );                // Compares tuples' ids

void char_to_tuple( char *str, tuple_t *tuple );        // Changes given buffer to tuple, also changes endianess hton

void tuple_to_char( tuple_t *tuple, char *str );        // Changes given tuple to characer buffer, also changes endianess ntoh


void ts_out( tuple_t tuple );                                                              // Adds tuple to tuple_space

void ts_inp( tuple_t template, tuple_t *retrive_tuple, int *inp_result );                  // Retrives tuple from tuple_space without getting return value

void ts_rdp( tuple_t template, tuple_t *retrive_tuple, int *rdp_result );                  /* Retrives tuple from tuple_space, doesnt remove it and gets return value
                                                                                           of success or failure*/

#endif