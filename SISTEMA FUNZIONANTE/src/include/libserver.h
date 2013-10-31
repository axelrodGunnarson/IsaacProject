#ifndef LIBSERVER_H
#define LIBSERVER_H


#include <semaphore.h>

#include "libsocket.h"


#define PORT 6000
#define TRAINER_PORT 6001
#define TIMEOUT_TIME 3
#define MAX_MSG_SIZE 10000
#define UCICLE_TIME 150000
#define N_FLAGS 55
#define N_PLAYERS 11
#define MAX_TEAMNAME_LENGHT 7

#define SENSE_STEP_MS 150 // a vision message each 150 ms
#define SENSE_BODY_STEP_MS 100 //a sense body message each 100 ms
#define SIMULATION_STEP_MS 100 //Used to send actions


extern sem_t conn;

/*Waits for the server to send a message (blocking possibly forever), reads it and puts it in buffer*/
int receiveMsg(server_data *server, char* buffer);

/* Just like receiveMsg, but wait for a given time */
int receiveTMsg(server_data *server, char* buffer, struct timeval *t);

/*Sends the given message to the server*/
int sendMsg(server_data *server, char* buffer);

/* try to connect to the server, sending an invalid token. If no answer is received
 * or the same token is received back, return an error */
int tryConnection(server_data *server);

/* send the conclusive token and destroy the connection */
int closeConnection(server_data *server);

#endif
