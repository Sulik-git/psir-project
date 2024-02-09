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
#define ID 1
#define FIELDS_NUM 2


typedef struct
{
    long is_actual;
    long type;
    union
    {
        long int_field;
        float float_field;
    } data;
    long padding;
    
} field_t;


typedef struct
{
    char name[NAME_MAX_SIZE];
    long tuple_len;
    field_t tuple_fields[TUPLE_MAX_SIZE];

} tuple_t ; //52B


void char_to_tuple( char *str, tuple_t *tuple );
void tuple_to_char( tuple_t *tuple, char *str );


void ts_out( tuple_t tuple );                                   // Adds tuple to tuple_space
tuple_t ts_inp( tuple_t template_inp ); // Retrives tuple from tuple_space without getting return value
tuple_t ts_rdp( tuple_t template_rdp);


struct alp_header
{
    uint8_t payload_type : 2;
    uint16_t sequence_number : 12;
    uint8_t rdp_result : 1;
    uint8_t acknowledge : 1;
};


struct payload
{
    uint8_t payload : 8;
};


struct alp_message
{
    struct alp_header header;
    struct payload payload[PAYLOAD_SIZE];
};


void reverse_chars( char *chars_to_reverse, int chars_num );
void prepare_alp_message( unsigned char *message, struct alp_message *msg_to_send, int operation );
void alp_init( byte *mac, int local_port );
void alp_send( unsigned char *message, int operation );
int alp_recv( unsigned char *recv_message, int* rdp_result );

int check_if_prime( int32_t number );


byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};


ZsutIPAddress server_ip=ZsutIPAddress(192,168,56,104);
ZsutEthernetUDP Udp;


tuple_t check_if_prime_template;
tuple_t result;
char str_result_prime[] = "is_prime";
char str_result_not_prime[] = "is_not_prime";


void setup() 
{
  char str_check[] = "check_if_prime";


  Serial.begin( 9600 );
  
  
  alp_init( mac, LOCAL_PORT );
  
  //Preparing check_if_prime template
  memcpy( (void *)check_if_prime_template.name, (void *)str_check, strlen(str_check) );
  check_if_prime_template.tuple_fields[0].is_actual = TS_YES;
  check_if_prime_template.tuple_fields[0].type = TS_INT;
  check_if_prime_template.tuple_fields[0].data.int_field = ID;
  check_if_prime_template.tuple_fields[1].is_actual = TS_NO;
  check_if_prime_template.tuple_fields[1].type = TS_INT;
  check_if_prime_template.tuple_len = FIELDS_NUM;

}

void loop() 
{

  Serial.print( "\n--WORKER_CYCLE--\n\n");
  Serial.print( "Trying to remove tuple \"check_if_prime\" \n" );


  result = ts_inp( check_if_prime_template );
  if ( result.tuple_fields[1].data.int_field != 0 )
  {
    Serial.print( "Tuple removed\n\n");
    Serial.print( "Checking if number is prime...\n" );
    if( check_if_prime(result.tuple_fields[1].data.int_field) )
    {
      
      Serial.print("Number is a prime!\n\n");


      memset( result.name, '\0', 16 * sizeof(char) );
      memcpy( (void *)result.name, (void *)str_result_prime, strlen(str_result_prime) );


      delay( 1000 );
      Serial.print("Sending result...\n");
      ts_out( result );

    }
    else
    {

      Serial.print("Number is not a prime!\n\n");


      memset( result.name, '\0', 16 * sizeof(char) );
      memcpy( (void *)result.name, (void *)str_result_not_prime, strlen(str_result_not_prime) );


      delay( 1000 );
      Serial.print( "Sending result...\n" );
      ts_out( result );

    }

    Serial.print( "Result sent\n" );

  }

  Serial.print(" \n ");
  

  delay( 1000 );

}

int check_if_prime( int32_t number)
{

  int counter = 0;


  for( int32_t n = 1; n <= number; n++)
  {

    if ( number % n == 0)
    {

      counter++;

    }

  }


  if ( counter <= 2)
  {
    return 1;
  }
  else
  {
    return 0;
  }

}

void reverse_chars(char *chars_to_reverse, int chars_num)
{

    for (int i = 0; i < chars_num / 2; i++)
    {

        char buff = '\0';


        buff = chars_to_reverse[chars_num - 1 - i];
        chars_to_reverse[chars_num - 1 - i] = chars_to_reverse[i];
        chars_to_reverse[i] = buff;

    }

}

void reverse_nonchars_vars(char *payload)
{

    int i = 16;


    for( i; i < 116; i = i + 4 )
    {
        reverse_chars( payload + i, sizeof(int) );
    }

}

void alp_init( byte *mac, int local_port )
{

    ZsutEthernet.begin( mac );
    Serial.print( ZsutEthernet.localIP() );
    Serial.print( "\n" );
    Udp.begin( local_port );

}

void alp_send( unsigned char *message, int operation )
{
  struct alp_message msg;
  unsigned char send_buff[ALP_MESSAGE_MAXSIZE] = {0}; 
  unsigned char recv_buff[ALP_MESSAGE_MAXSIZE] = {0};
  int start_time = 0;
  int stop_time = 0;
  int packetSize = 0;

  //Serial.print( "ALP: Preparing ALP message...\n" );
  prepare_alp_message( message, &msg, operation );
  memcpy( (void *)send_buff, (void *)&msg, ALP_MESSAGE_MAXSIZE ); 
  //Serial.print( "ALP: Message prepared\n" );

  start_time = ZsutMillis();


  Udp.beginPacket( server_ip, SERVER_PORT );
  Udp.write( send_buff, ALP_MESSAGE_MAXSIZE );
  Udp.endPacket();


  //Serial.print("ALP: Sent Payload message - waiting for ACK\n");
  

  while( 1 )
  {

    packetSize = Udp.parsePacket();
    if( packetSize )
    {

      Udp.read( recv_buff, ALP_MESSAGE_MAXSIZE );


      memcpy( (void *)&msg, (void *)recv_buff, ALP_MESSAGE_MAXSIZE );
      if( msg.header.acknowledge & ALP_BIN_ACK_FLAG )
      {

        //Serial.print("ALP: Received Ack\n");
        break;

      }


      stop_time = ZsutMillis();


      if( (stop_time - start_time) >= TIMEOUT_MILISEC )
      {

        Udp.beginPacket( server_ip, SERVER_PORT );
        Udp.write( send_buff, ALP_MESSAGE_MAXSIZE );
        Udp.endPacket();


        //Serial.print("ALP: Resent Payload message\n");


        start_time = ZsutMillis();

      }
    }  
  }
}

int alp_recv( unsigned char *recv_message, int *rdp_result )
{

  struct alp_message msg;
  unsigned char recv_buffor[ALP_MESSAGE_MAXSIZE] = {0};
  unsigned char send_buffor[ALP_MESSAGE_MAXSIZE] = {0};
  int packetSize = 0;


  prepare_alp_message( NULL, &msg, ALP_ACK_OPERATION );
  memcpy( (void *)send_buffor, (void *)&msg, ALP_MESSAGE_MAXSIZE );

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
  
  //for( int i=0; i < ALP_MESSAGE_MAXSIZE; i++ )
  //{

  //  Serial.print( recv_buffor[i], HEX );
  //  Serial.print(" ");
  
  //}
  //Serial.print( "\n" );


  memcpy( (void *)&msg, (void *)recv_buffor, ALP_MESSAGE_MAXSIZE );  
  memcpy( (void *)recv_message, (void *)&msg.payload, PAYLOAD_SIZE );
  
  
  return packetSize;

}


void prepare_alp_message( unsigned char *message, struct alp_message *msg_to_send, int operation )
{

    memset( msg_to_send, 0, sizeof(struct alp_message) );


    if ( operation == ALP_ACK_OPERATION )
    {
        
        msg_to_send->header.acknowledge = ALP_BIN_ACK_FLAG;
        msg_to_send->header.payload_type = ALP_BIN_ACK_PAYLOAD;
        msg_to_send->header.sequence_number = 0b0;

    }
    if ( operation == ALP_OUT_OPERATION )
    {
        
        msg_to_send->header.acknowledge = 0b0;
        msg_to_send->header.payload_type = ALP_BIN_OUT_PAYLOAD;
        msg_to_send->header.sequence_number = 0b0;


        memcpy( (void *)&msg_to_send->payload, (void *)message, PAYLOAD_SIZE );

    }
    if ( operation == ALP_RDP_OPERATION )
    {
        
        msg_to_send->header.acknowledge = 0b0;
        msg_to_send->header.payload_type = ALP_BIN_RDP_PAYLOAD;
        msg_to_send->header.sequence_number = 0b0;


        memcpy( (void *)&msg_to_send->payload, (void *)message, PAYLOAD_SIZE );

    }
    if ( operation == ALP_INP_OPERATION )
    {
        
        msg_to_send->header.acknowledge = 0b0;
        msg_to_send->header.payload_type = ALP_BIN_INP_PAYLOAD;
        msg_to_send->header.sequence_number = 0b0;


        memcpy( (void *)&msg_to_send->payload, (void *)message, PAYLOAD_SIZE );

    }


    return EXIT_SUCCESS;

}

void tuple_to_char( tuple_t *tuple, char *str )
{

    memcpy( (void *)str, (void *)tuple, sizeof(tuple_t) );

}

void char_to_tuple( char *str, tuple_t *tuple )
{

    long table[PAYLOAD_NONCHARS_SIZE / 4];


    memcpy( (void *)tuple, (void *)str, 16 * sizeof(unsigned char) ); //Copying name from buffor to tuple
    for( int i = 0; i < PAYLOAD_NONCHARS_SIZE / 4; i++ )
    {

        memcpy( (void *)&table[i], (void *)( str + sizeof(unsigned char) * 16 + i * sizeof(long) ), sizeof(long) );
        //Serial.print(" ");
        //Serial.print(table[i]);
    
    }
    memcpy( (void *)&tuple->tuple_len, (void *)table, PAYLOAD_NONCHARS_SIZE );

    //memcpy( (void *)tuple, (void *)str, sizeof(tuple_t) );

}

void ts_out( tuple_t tuple )
{

  unsigned char buffer[PAYLOAD_SIZE];


  memset( buffer, 0, PAYLOAD_SIZE );
  tuple_to_char( &tuple, buffer );
  alp_send( buffer, ALP_OUT_OPERATION );

}

tuple_t ts_inp( tuple_t template_inp )
{

  unsigned char buffer[PAYLOAD_SIZE] = {0};
  int rdp_result = 0;
  tuple_t recv_tuple;
 

  tuple_to_char( &template_inp, buffer );
  alp_send( buffer, ALP_INP_OPERATION );   
  

  memset( buffer, 0, PAYLOAD_SIZE );
 
  
  int r = alp_recv( buffer, &rdp_result );
   
     
  char_to_tuple( buffer, &recv_tuple );
  return recv_tuple; 
                            
}

tuple_t ts_rdp( tuple_t template_rdp )
{

  unsigned char buffer[PAYLOAD_SIZE] = {0};
  int rdp_result = 0;
  tuple_t recv_tuple;
 

  tuple_to_char( &template_rdp, buffer );
  alp_send( buffer, ALP_RDP_OPERATION );   
  

  memset( buffer, 0, PAYLOAD_SIZE );
 
  
  int r = alp_recv( buffer, &rdp_result );
   
     
  char_to_tuple( buffer, &recv_tuple );
  return recv_tuple; 
      
}

