#ifndef LIBSOCKET_H
#define LIBSOCKET_H

#include <netinet/in.h>
#include <semaphore.h>

#define MAX_MSG_LENGTH 5000


typedef struct Server_data{
	int sock;
	struct sockaddr_in sa;
} server_data; 


/*returns a socket connected to localhost:port, or -1 if the connection failed.*/
server_data *createServer( char* addr, int port);	

/* close server socket and free pointer memory */
int destroyServer(server_data *server);
/* send an UDP message */
int Send( server_data *server, char* buffer );

/* receive an UDP message, without timeout */
int Receive(server_data *server, char *buffer);

/* try to receive an UDP message, with timeout */
int TimeoutReceive(server_data *server, char*buffer, struct timeval *t);

#endif
