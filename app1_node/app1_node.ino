#include "ZsutEthernet.h"
#include "ZsutEthernetUdp.h"
#include "ZsutFeatures.h"
#include "TimerOne.h"
#define PAYLOAD_SIZE 148
#define ALP_MESSAGE_MAXSIZE 150
#define ALP_ACKNOWLEDGED 1
#define ALP_NOT_ACKNOWLEDGED 0
#define ALP_BIN_ACK_FLAG 0b1
#define ALP_BIN_ACK_PAYLOAD 0b00
#define ALP_BIN_OUT_PAYLOAD 0b01
#define ALP_BIN_RDP_PAYLOAD 0b10
#define ALP_BIN_INP_PAYLOAD 0b11
#define ALP_ACK_OPERATION 1
#define ALP_OUT_OPERATION 2
#define ALP_INP_OPERATION 3
#define ALP_RDP_OPERATION 4
#define SERVER_PORT 12345
#define LOCAL_PORT 11111
#define NAME_MAX_SIZE 16
#define TUPLE_MAX_SIZE 8
#define TS_YES 1
#define TS_NO 0
#define TS_INT 0
#define TS_FLOAT 1
#define TS_SUCCESS 1
#define TS_FAILURE 0
#define PAYLOAD_NONCHARS_SIZE 132
#define TIMEOUT_MILISEC 1000


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

} tuple_t  ;


void char_to_tuple(char *str, tuple_t *tuple);
void tuple_to_char(tuple_t *tuple, char *str);

void ts_out(tuple_t tuple);                            // Adds tuple to tuple_space
void ts_inp(tuple_t template_inp, tuple_t *retrive_tuple); // Retrives tuple from tuple_space without getting return value
void ts_rdp(tuple_t template_rdp, tuple_t *retrive_tuple);

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};


ZsutIPAddress server_ip=ZsutIPAddress(127,0,0,1);

ZsutEthernetUDP Udp;

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

void reverse_chars(char *chars_to_reverse, int chars_num);
void prepare_alp_message(char *message,struct alp_message *msg_to_send,int operation);
void alp_init(byte *mac,int local_port);
void alp_send(char *message, int operation);
int alp_recv(char *recv_message);

tuple_t test;
tuple_t rdp_test;
tuple_t *rdp_test_p = &rdp_test;
char rdp_test_msg[128];

void setup() 
{
  Serial.begin(9600);
  alp_init(mac,LOCAL_PORT);
  
  
  char str[] = "test";
  memset(rdp_test_msg,'\0',128);
  memset(test.name,0,NAME_MAX_SIZE);
  memset(&rdp_test,0,sizeof(tuple_t));
  memcpy((void *)test.name,(void *)str,sizeof(str));
  test.tuple_fields[0].is_actual = 1;
  test.tuple_fields[0].type = TS_INT;
  test.tuple_fields[0].data.int_field = 5;
  test.tuple_fields[0].padding = 0;
  test.tuple_fields[1].is_actual = 1;
  test.tuple_fields[1].type = TS_INT;
  test.tuple_fields[1].data.int_field = 20;
  test.tuple_fields[1].padding = 0;
  test.tuple_len = 2;

  

}

void loop() 
{
  ts_out(test);
  delay(1000);
  ts_rdp(test,rdp_test_p);
  if(rdp_test_p == NULL)
  {
    rdp_test_p = &rdp_test;
  }
  else
  {
    sprintf(rdp_test_msg,"Received tuple named: %s with field1: %d and field2: %d",rdp_test.name,rdp_test.tuple_fields[0].data,rdp_test.tuple_fields[1].data);
    Serial.println(rdp_test_msg);
    
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
    int i=16;
    for(i;i<116;i = i + 4)
    {
        reverse_chars(payload + i,sizeof(int));
    }
}

void alp_init(byte *mac,int local_port)
{
    ZsutEthernet.begin(mac);
    Serial.println(ZsutEthernet.localIP());
    Udp.begin(local_port);
}

void alp_send(char *message,int operation)
{
  int ack = 0;
  struct alp_message send_msg;
  struct alp_message recv_msg;
  char buff[ALP_MESSAGE_MAXSIZE];
  int start_time = 0;
  int stop_time = 0;
  prepare_alp_message(message,&send_msg,operation);
  memcpy((void *)buff,(void *)&send_msg,ALP_MESSAGE_MAXSIZE);  
  start_time = ZsutMillis();
  Udp.beginPacket(server_ip,SERVER_PORT);
  int w=Udp.write(buff,ALP_MESSAGE_MAXSIZE);
  Udp.endPacket();
  int packetSize = 0;
  while(ack == ALP_NOT_ACKNOWLEDGED)
  {
    packetSize = Udp.parsePacket();
    if(packetSize)
    {
      int r = Udp.read(message,ALP_MESSAGE_MAXSIZE);
      memcpy((void *)&recv_msg,(void *)message,ALP_MESSAGE_MAXSIZE);
      if(recv_msg.header.acknowledge & ALP_BIN_ACK_FLAG)
      {
        ack = ALP_ACKNOWLEDGED;
        continue;
      }
      stop_time = ZsutMillis();

      if((stop_time - start_time)>=TIMEOUT_MILISEC)
      {
        Udp.beginPacket(server_ip,SERVER_PORT);
        int r=Udp.write(buff,ALP_MESSAGE_MAXSIZE);
        Udp.endPacket();
        start_time = ZsutMillis();
      }

    }  
  }

}

int alp_recv(char *recv_message,int *rdp_result)
{
  struct alp_message ack_msg;
  struct alp_message alp_recv_msg;
  char buffor[ALP_MESSAGE_MAXSIZE];
  int packetSize = 0;

  prepare_alp_message(NULL,&ack_msg,ALP_ACK_OPERATION);
  packetSize = Udp.parsePacket();
  if(packetSize)
  {
    int r = Udp.read(buffor,ALP_MESSAGE_MAXSIZE);
    Udp.beginPacket(server_ip,SERVER_PORT);
    int w=Udp.write((char *)&ack_msg,ALP_MESSAGE_MAXSIZE);
    Udp.endPacket();
    for(int i=0;i<150;i++)
    {
      Serial.print(buffor[i],HEX);
    }
    Serial.println();
    
    memcpy((void *)&alp_recv_msg,(void *)buffor,ALP_MESSAGE_MAXSIZE);
    memcpy((void *)recv_message,(void *)&alp_recv_msg.payload,PAYLOAD_SIZE);
    
  }
  return packetSize;
}


void prepare_alp_message(char *message,struct alp_message *msg_to_send,int operation)
{
    memset(msg_to_send, 0, sizeof(struct alp_message));
    if (operation == ALP_ACK_OPERATION)
    {
        
        msg_to_send->header.acknowledge = ALP_BIN_ACK_FLAG;
        msg_to_send->header.payload_type = ALP_BIN_ACK_PAYLOAD;
        msg_to_send->header.sequence_number = 0b0;
    }
     if (operation == ALP_RDP_OPERATION)
    {
        
        msg_to_send->header.acknowledge = 0b0;
        msg_to_send->header.payload_type = ALP_BIN_RDP_PAYLOAD;
        msg_to_send->header.sequence_number = 0b0;
        memcpy((void *)&msg_to_send->payload,(void *)message,PAYLOAD_SIZE);
    }
     if (operation == ALP_OUT_OPERATION)
    {
        
        msg_to_send->header.acknowledge = 0b0;
        msg_to_send->header.payload_type = ALP_BIN_OUT_PAYLOAD;
        msg_to_send->header.sequence_number = 0b0;
        memcpy((void *)&msg_to_send->payload,(void *)message,PAYLOAD_SIZE);
    }
     if (operation == ALP_INP_OPERATION)
    {
        
        msg_to_send->header.acknowledge = 0b0;
        msg_to_send->header.payload_type = ALP_BIN_INP_PAYLOAD;
        msg_to_send->header.sequence_number = 0b0;
        memcpy((void *)&msg_to_send->payload,(void *)message,PAYLOAD_SIZE);
    }
    return EXIT_SUCCESS;
}

void tuple_to_char(tuple_t *tuple, char *str)
{
    memcpy((void *)str, (void *)tuple, sizeof(tuple_t));
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

void ts_out(tuple_t tuple)
{
  char buffer[PAYLOAD_SIZE];
  memset(buffer,0,PAYLOAD_SIZE);
  tuple_to_char(&tuple,buffer);
  alp_send(buffer,ALP_OUT_OPERATION);
}

void ts_inp(tuple_t template_inp, tuple_t *retrive_tuple)
{
  char buffer[PAYLOAD_SIZE];
  tuple_t recv_tuple;
  memset(buffer,0,PAYLOAD_SIZE);
  tuple_to_char(&template_inp,buffer);
  alp_send(buffer,ALP_INP_OPERATION);
  memset(buffer,0,PAYLOAD_SIZE);
  while(1)
  {
    
    if(alp_recv(buffer))       // Checking if this is the tuple that we wanted or that this is tuple at all or smth
    {
      char_to_tuple(buffer,&recv_tuple);
      *retrive_tuple = recv_tuple;
      break; 
    } 
  }
                            
}

void ts_rdp(tuple_t template_rdp, tuple_t *retrive_tuple)
{
  char buffer[PAYLOAD_SIZE];
  int rdp_result = 0;
  tuple_t recv_tuple;
  memset(buffer,0,PAYLOAD_SIZE);
  tuple_to_char(&template_rdp,buffer);
  alp_send(buffer,ALP_INP_OPERATION);
  memset(buffer,0,PAYLOAD_SIZE);
  while(1)
  {
    int r = alp_recv(buffer,&rdp_result);
    if(r)       // Checking if this is the tuple that we wanted or that this is tuple at all or smth
    {
      if(rdp_result)
      {
        Serial.println("Im here");
        char_to_tuple(buffer,&recv_tuple);
        *retrive_tuple = recv_tuple;
        break; 
      }
      else
      {
        retrive_tuple = NULL;
        break;
      }
    } 
  }
}

