CC=gcc
FLAGS= -g -o
O_FLAGS=-g -c -o
OBJECTS= obj/tuple_space_API.o obj/tuple_space_linked_list.o obj/application_layer_protocol.o obj/alp_error.o obj/timer.o
HEADERS= headers/alp_error.h headers/application_layer_protocol.h headers/tuple_space_API.h headers/tuple_space_linked_list.h headers/timer.h
SERVER=bin/server
SERVER_OBJ=obj/server.o

all: ${SERVER}

${SERVER}: ${OBJECTS} ${HEADERS} ${SERVER_OBJ}
	${CC} ${FLAGS} ${SERVER} ${SERVER_OBJ} ${OBJECTS}
	
obj/%.o:src/%.c
	${CC} ${O_FLAGS} $@ $^

clean:
	rm -rf ${OBJECTS} ${SERVER_OBJ} ${SERVER} ${TEST}