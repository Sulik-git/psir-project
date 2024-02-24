#include "ZsutEthernet.h"
#include "ZsutEthernetUdp.h"
#include "ZsutFeatures.h"
#include "TimerOne.h"
#define PAYLOAD_SIZE 52
#define ALP_MESSAGE_MAXSIZE 54
#define ALP_ACKNOWLEDGED 1
#define ALP_NOT_ACKNOWLEDGED 0
#define ALP_BIN_ACK_FLAG 0b1
#define ALP_BIN_ACK_PAYLOAD 0b00
#define ALP_BIN_OUT_PAYLOAD 0b01
#define ALP_BIN_RDP_PAYLOAD 0b10
#define ALP_BIN_INP_PAYLOAD 0b11
#define ALP_ACK_OPERATION 0
#define ALP_OUT_OPERATION 1
#define ALP_RDP_OPERATION 2
#define ALP_INP_OPERATION 3
#define SERVER_PORT 12345
#define LOCAL_PORT 11111
#define NAME_MAX_SIZE 16
#define TUPLE_MAX_SIZE 2
#define TS_YES 1
#define TS_NO 0
#define TS_INT 0
#define TS_FLOAT 1
#define TS_SUCCESS 1
#define TS_FAILURE 0
#define PAYLOAD_NONCHARS_SIZE 36
#define TIMEOUT_MILISEC 1000
#define FIELDS_NUM 2
#define N 5
#define ID 2
#define __bin_to_number(x) ( (x & 0b10000000000000) >> 13 )

typedef struct
{
    int32_t is_actual;                // 1 if field is actual field or 0 when field is any value with specific type
    int32_t type;                     // 1 if field is float type or 0 when field is int type 
    union                             
    {
        int32_t int_field;
        float float_field;
    } data;                           // Union containing data
    int32_t padding;                  // Padding added so field struct can be 16 Bytes 
    
} field_t;    // Struct that represents tuple's field


typedef struct
{
    char name[NAME_MAX_SIZE];                   // Tuple name
    int32_t fields_num;                          // Number of tuple's fields
    field_t tuple_fields[TUPLE_MAX_SIZE];       // Tuple's fields array

} tuple_t;    // Struct that represents tuple


typedef struct                        // Same as field_t but with uint32 in every field of struct
{
    uint32_t is_actual;
    uint32_t type;
    union {
        uint32_t int_field;
        uint32_t float_field;
    } data;
    uint32_t padding;

}field_n_t;   // Network field struct used for hton and ntoh (changing endianess operations)


typedef struct 
{
    char name[NAME_MAX_SIZE];
    uint32_t field_num;
    field_n_t field[TUPLE_MAX_SIZE];

} tuple_n_t;   // Network tuple struct used for hton and ntoh (changing endianess operations)


void char_to_tuple( char *str, tuple_t *tuple );    // Changes given character buffer to tuple
void tuple_to_char( tuple_t *tuple, char *str );    // Changes given tuple struct to proper character buffer


void ts_out( tuple_t tuple );                            // Adds tuple to tuple_space
tuple_t ts_inp( tuple_t template_inp, int *inp_result ); // Retrives tuple from tuple_space with getting return value and removes it
tuple_t ts_rdp( tuple_t template_rdp, int *rdp_result ); // Retrives tuple from tuple_space with getting return value


struct alp_header                                     // ALP header struct
{
    uint8_t payload_type : 2;                         // Payload type = 2 bits; 00 - ack payload, 01 - out payload, 10 - rdp payload, 11 - inp payload             
    uint16_t sequence_number : 12;                    // Sequence number = 12 bits; NOT IMPLEMENTED
    uint8_t op_result : 1;                            // Operation result = 1 bit; 0 - Tuple not present in tuple_space, 1 - Tuple present in tuple_space and retrived
    uint8_t acknowledge : 1;                          // Acknowledge = 1 bit; 1 - Alp message is ack message
} __attribute__((packed));


struct alp_message                                    // Alp message struct
{
    struct alp_header header;                         // Alp header
    uint8_t payload[PAYLOAD_SIZE];                    // Array of uint8_t containing payload
} __attribute__((packed));

struct alp_message_n
{
  uint16_t header;
  uint8_t payload[PAYLOAD_SIZE];
} __attribute__((packed));


void prepare_alp_message( unsigned char *payload, struct alp_message *msg_to_send, int operation );            // Prepare alp message with given payload and operation

void alp_init( byte *mac, int local_port );                                                                    // Initialize MAC and local port

void alp_send( unsigned char *payload, int operation );                                                         // Sends alp message with given payload

int alp_recv( unsigned char *recv_payload, int *op_result );                                                   // Receives alp message, puts received payload to given buffer and sets operation result


byte mac[] = {                                                                                                  // Arduino MAC
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};


ZsutIPAddress server_ip=ZsutIPAddress( 192, 168, 56, 104 );
ZsutEthernetUDP Udp;


tuple_t tuple_change;
int op_result;
int old_sensor_value;
int sensor_value;


void setup() 
{

  char str_tuple_changed[] = "change_tuple";


  ZsutPinMode( ZSUT_PIN_D13, INPUT ); 


  Serial.begin( 9600 );
  
  
  alp_init( mac, LOCAL_PORT );


  //Preparing tuple "change_tuple" 
  memcpy( (void *)tuple_change.name, (void *)str_tuple_changed, strlen(str_tuple_changed) );
  tuple_change.fields_num = FIELDS_NUM;
  tuple_change.tuple_fields[0].is_actual = TS_YES;
  tuple_change.tuple_fields[0].type = TS_INT;
  tuple_change.tuple_fields[0].data.int_field = ID;
  tuple_change.tuple_fields[1].is_actual = TS_YES;
  tuple_change.tuple_fields[1].type = TS_INT;


  old_sensor_value = ZsutDigitalRead(); // First sensor read for future comparing
  

}


void loop() 
{

  delay(2000);


  Serial.print("\n--SENSOR_CYCLE--\n");
  Serial.print("Sensing...\n");


  sensor_value = __bin_to_number(ZsutDigitalRead());  // Converting sensor bin value to integer
  //Serial.print( sensor_value, BIN );
  //Serial.print( " \n " );
  if( sensor_value != old_sensor_value )        // Checking if sensed value has changed
  {

    Serial.print( "Change sensed!!!\n" );

    // Assigning new sensor value which is 1 or 0. This indicates which change occured ( 0->1 or 1->0 ).
    tuple_change.tuple_fields[1].data.int_field = sensor_value;
    ts_out( tuple_change );                                       // Sending tuple


    Serial.print("Change tuple OUT!!!\n");

  }


  old_sensor_value = sensor_value; // Saving new value as last that occured


}









// ALP SECTION


uint32_t htonl( uint32_t hostlong )             // Returns uint32 with changed endian from Little Endian to Big Endian
{
    return ( (hostlong & 0x000000FF) << 24 ) |
           ( (hostlong & 0x0000FF00) << 8 ) |
           ( (hostlong & 0x00FF0000) >> 8 ) |
           ( (hostlong & 0xFF000000) >> 24 );
}



uint32_t ntohl( uint32_t netlong )              // Returns uint32 with changed endian from Big Endian to Little Endian
{
    return ( (netlong & 0x000000FF) << 24 ) |
           ( (netlong & 0x0000FF00) << 8 ) |
           ( (netlong & 0x00FF0000) >> 8 ) |
           ( (netlong & 0xFF000000) >> 24 );
}

uint16_t ntohs(uint16_t netshort) 
{

    return ((netshort & 0xFF00) >> 8) | ((netshort & 0x00FF) << 8);

}

uint16_t htons(uint16_t netshort) 
{

    return ((netshort & 0xFF00) >> 8) | ((netshort & 0x00FF) << 8);

}


void alp_header_ntoh( struct alp_message_n *recv_network_message, struct alp_message *recv_message ) // Changes endianess in alp_header from network to host
{

    recv_network_message->header = ntohs(recv_network_message->header);


    memcpy( (void *)&recv_message->header, (void *)&recv_network_message->header, sizeof(uint16_t) );

}


void alp_header_hton( struct alp_message *message, struct alp_message_n *network_message ) // Changes endianess in alp_header from host to network
{

    memcpy( (void *)&network_message->header, (void *)&message->header, sizeof(uint16_t) );


    network_message->header = htons(network_message->header);

}


void field_hton( field_n_t *field )              // Changes endianess of every element of field_t struct from host to network
{

    field->is_actual = htonl( field->is_actual );
    field->type = htonl( field->type );


    // Changes only one field of union depending on type
    if( field->type == TS_INT )
    {

      field->data.int_field = htonl( field->data.int_field );

    } else if( field->type == TS_FLOAT)
    {

      field->data.float_field = htonl( field->data.float_field );

    }

}



void field_ntoh( field_n_t *field )             // Changes endianess of every element of field_t struct from network to host
{

    field->is_actual = ntohl( field->is_actual );
    field->type = ntohl( field->type );


    // Changes only one field of union depending on type
    if( field->type == TS_INT )
    {

      field->data.int_field = ntohl ( field->data.int_field );

    } else if( field->type == TS_FLOAT)
    {

      field->data.float_field = ntohl ( field->data.float_field );

    }

}



void alp_init( byte *mac, int local_port )          // Initializes MAC and local port
{

    ZsutEthernet.begin( mac );
    Serial.print( ZsutEthernet.localIP() );
    Serial.print( "\n\n" );
    Udp.begin( local_port );

}



void alp_send( unsigned char *payload, int operation )    // Sends alp message with given payload
{

  
  struct alp_message_n network_msg;
  struct alp_message msg;
  unsigned char send_buff[ALP_MESSAGE_MAXSIZE] = {0}; 
  unsigned char recv_buff[ALP_MESSAGE_MAXSIZE] = {0};
  int start_time = 0;
  int stop_time = 0;
  int packetSize = 0;


  memset( (void *)&msg, 0, sizeof(struct alp_message) );

  //Prepares alp message for sending

    //Serial.print( "ALP: Preparing ALP message...\n" );
  prepare_alp_message( payload, &network_msg, operation );
  memcpy( (void *)send_buff, (void *)&network_msg, ALP_MESSAGE_MAXSIZE );
    //Serial.print( "ALP: Message prepared\n" );



  // Gets time from ZsutMilis to calculate not receiving ack timeout
  start_time = ZsutMillis();


  // Sends first alp message
  Udp.beginPacket( server_ip, SERVER_PORT );
  Udp.write( send_buff, ALP_MESSAGE_MAXSIZE );
  Udp.endPacket();


    //Serial.print( "ALP: Sent Payload message - waiting for ACK\n" );
  

  // Looping unless receive ACK
  while( 1 )
  {

    packetSize = Udp.parsePacket();
    if( packetSize )
    {

      Udp.read( recv_buff, ALP_MESSAGE_MAXSIZE );


      // Copying memory from received buffer to alp message struct and checking ACK flag 
      memset( (void *)&network_msg, 0, sizeof(struct alp_message_n) );
      memcpy( (void *)&network_msg, (void *)recv_buff, ALP_MESSAGE_MAXSIZE );

      alp_header_ntoh( &network_msg, &msg );

      if( msg.header.acknowledge & ALP_BIN_ACK_FLAG )
      {

        //Serial.print("ALP: Received Ack\n");
        break;

      }

      // Getting second measure of time
      stop_time = ZsutMillis();


      // Checking for timeout and resending message in case of one
      if( (stop_time - start_time) >= TIMEOUT_MILISEC )
      {

        Udp.beginPacket( server_ip, SERVER_PORT );
        Udp.write( send_buff, ALP_MESSAGE_MAXSIZE );
        Udp.endPacket();


          //Serial.print("ALP: Resent Payload message\n");


        // Reseting timeout first measure
        start_time = ZsutMillis();

      }
    }  
  }
}



int alp_recv( unsigned char *recv_payload, int *op_result )
{

  struct alp_message_n network_msg;
  struct alp_message msg;
  unsigned char recv_buffor[ALP_MESSAGE_MAXSIZE] = {0};
  unsigned char send_buffor[ALP_MESSAGE_MAXSIZE] = {0};
  int packetSize = 0;


  memset( (void *)&msg, 0, sizeof(struct alp_message) );

  //Prepares ACK alp message for sending
  prepare_alp_message( NULL, &network_msg, ALP_ACK_OPERATION );
  memcpy( (void *)send_buffor, (void *)&network_msg, ALP_MESSAGE_MAXSIZE );


  // Looping unless receiving something
  while( 1 )
  {

    packetSize = Udp.parsePacket();
    if( packetSize )
    {

      Udp.read( recv_buffor, ALP_MESSAGE_MAXSIZE );


      Udp.beginPacket( server_ip, SERVER_PORT );
      Udp.write( send_buffor, ALP_MESSAGE_MAXSIZE );
      Udp.endPacket();


      break;
    
    }
  }
  

  // Copying memory from received buffer to alp message struct and then from struct alp_message payload to given to function buffer 
  memcpy( (void *)&network_msg, (void *)recv_buffor, ALP_MESSAGE_MAXSIZE );
  alp_header_ntoh( &network_msg, &msg );
  *op_result = msg.header.op_result;                                          // Setting operation result
  memcpy( (void *)recv_payload, (void *)&network_msg.payload, PAYLOAD_SIZE );
  
  
  return packetSize;

}



void prepare_alp_message( unsigned char *payload, struct alp_message_n *msg_to_send, int operation )    // Prepare alp message with given payload and operation
{

    struct alp_message temp_msg;
    // Zeroing memory of given struct
    memset( (void *)msg_to_send, 0, sizeof(struct alp_message_n) );
    memset( (void *)&temp_msg, 0, sizeof(struct alp_message) );


    // Checking for operation type, setting every alp_header field accordingly and copying payload form given buffer to msg_to_send struct
    if ( operation == ALP_ACK_OPERATION )
    {
        
        temp_msg.header.acknowledge = ALP_BIN_ACK_FLAG;
        temp_msg.header.payload_type = ALP_BIN_ACK_PAYLOAD;
        temp_msg.header.sequence_number = 0b0;


        alp_header_hton( &temp_msg, msg_to_send );

    }
    if ( operation == ALP_OUT_OPERATION )
    {
        
        temp_msg.header.acknowledge = 0b0;
        temp_msg.header.payload_type = ALP_BIN_OUT_PAYLOAD;
        temp_msg.header.sequence_number = 0b0;


        alp_header_hton( &temp_msg, msg_to_send );
        memcpy( (void *)&msg_to_send->payload, (void *)payload, PAYLOAD_SIZE );

    }
    if ( operation == ALP_RDP_OPERATION )
    {
        
        temp_msg.header.acknowledge = 0b0;
        temp_msg.header.payload_type = ALP_BIN_RDP_PAYLOAD;
        temp_msg.header.sequence_number = 0b0;


        alp_header_hton( &temp_msg, msg_to_send );
        memcpy( (void *)&msg_to_send->payload, (void *)payload, PAYLOAD_SIZE );

    }
    if ( operation == ALP_INP_OPERATION )
    {
        
        temp_msg.header.acknowledge = 0b0;
        temp_msg.header.payload_type = ALP_BIN_INP_PAYLOAD;
        temp_msg.header.sequence_number = 0b0;


        alp_header_hton( &temp_msg, msg_to_send );
        memcpy( (void *)&msg_to_send->payload, (void *)payload, PAYLOAD_SIZE );

    }


    return EXIT_SUCCESS;

}



void tuple_to_char( tuple_t *tuple, char *str )                         // Changes given tuple struct to proper character buffer      
{

    tuple_n_t network_tuple;


    //Copies values of each element of tuple to network tuple and then changes endianess from host to network
    memcpy( network_tuple.name, &tuple->name, NAME_MAX_SIZE );
    network_tuple.field_num = (uint32_t)tuple->fields_num;
    for( size_t s = 0; s < TUPLE_MAX_SIZE; s++ )
    {

        network_tuple.field[s].is_actual = (uint32_t)tuple->tuple_fields[s].is_actual;
        network_tuple.field[s].type = (uint32_t)tuple->tuple_fields[s].type;
        if( tuple->tuple_fields[s].type == TS_INT )
        {

          network_tuple.field[s].data.int_field = (uint32_t)tuple->tuple_fields[s].data.int_field;

        }
        else if( tuple->tuple_fields[s].type == TS_FLOAT )
        {

          memcpy( &network_tuple.field[s].data.float_field, &tuple->tuple_fields[s].data.float_field, sizeof(float) );

        }
        network_tuple.field[s].padding = (uint32_t)tuple->tuple_fields[s].padding;


        field_hton( &network_tuple.field[s] );

    }
    network_tuple.field_num = htonl( network_tuple.field_num );


    //Copies bytes from network tuple to char buffer
    memcpy( (void *)str, (void *)&network_tuple, sizeof(tuple_n_t) );

}



void char_to_tuple( char *str, tuple_t *tuple )                                     // Changes given character buffer to tuple
{

    tuple_n_t network_tuple;


    memcpy( (void *)&network_tuple, (void *)str, sizeof(tuple_t) );                 //Copies whole buffer to tuple to fill out name field
    memcpy( (void *)&tuple->name, (void *)&network_tuple , NAME_MAX_SIZE );

    // Changes endianess for every tuple's field
    for( size_t s = 0; s < TUPLE_MAX_SIZE; s++)
    {

        field_ntoh( &network_tuple.field[s] ); 
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


    // Changes endian of field_num and then assigns it to given tuple
    network_tuple.field_num = ntohl( network_tuple.field_num );
    tuple->fields_num = (int32_t)network_tuple.field_num;

}



void ts_out( tuple_t tuple )              // Out operation - converting tuple to chararacters and copying content to buffer, sending buffer 
{

  unsigned char buffer[PAYLOAD_SIZE] = {0};


  tuple_to_char( &tuple, buffer );
  alp_send( buffer, ALP_OUT_OPERATION );

}



tuple_t ts_inp( tuple_t template_inp, int *inp_result )     // Inp operation - converting tuple to characters and cpying content to buffer, sending buffer, zeroing buffer,
{                                                           // waiting to receive content and set inp operation result, converting characters from buffer to tuple, returning tuple

  unsigned char buffer[PAYLOAD_SIZE] = {0};
  tuple_t recv_tuple;
 

  tuple_to_char( &template_inp, buffer );
  alp_send( buffer, ALP_INP_OPERATION );   
  

  memset( buffer, 0, PAYLOAD_SIZE );
 
  
  int r = alp_recv( buffer, inp_result );
   
     
  char_to_tuple( buffer, &recv_tuple );
  return recv_tuple; 
                            
}



tuple_t ts_rdp( tuple_t template_rdp, int *rdp_result )        // Rdp operation - converting tuple to characters and cpying content to buffer, sending buffer, zeroing buffer,
{                                                              // waiting to receive content and set rdp operation result, converting characters from buffer to tuple, returning tuple 

  unsigned char buffer[PAYLOAD_SIZE] = {0};
  tuple_t recv_tuple;
 

  tuple_to_char( &template_rdp, buffer );
  alp_send( buffer, ALP_RDP_OPERATION );   
  

  memset( buffer, 0, PAYLOAD_SIZE );
 
  
  int r = alp_recv( buffer, rdp_result );
   
     
  char_to_tuple( buffer, &recv_tuple );
  return recv_tuple; 
      
}