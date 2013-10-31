#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>


#include "../include/errlib.h"
#include "../include/libsocket.h"


server_data *createServer( char* addr, int port){
	server_data *server;
	server=(server_data*)malloc(sizeof(server_data));
	memset(server,0,sizeof(server_data));

	/* creating socket */
	server->sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == server->sock)
		err_msg("Error Creating Socket");

	/* setting server parameters */
	server->sa.sin_family = AF_INET;
	server->sa.sin_addr.s_addr = inet_addr(addr);
	server->sa.sin_port = htons(port);

	/* binding socket to the port, in order to receive data */
	bind( server->sock, (struct sockaddr *)&(server->sa), sizeof(struct sockaddr_in) );

	return server;
}


int Send( server_data *server, char* buffer ){
	int bytes_sent;

	bytes_sent = sendto(server->sock, buffer, strlen(buffer), 0,(struct sockaddr*)&(server->sa), sizeof(server->sa));
	if (bytes_sent < 0)
		err_quit("Error sending packet");
	return bytes_sent;
}

int Receive(server_data *server, char *buffer){
	int bytes_received=0;
	socklen_t len;

	len=sizeof(struct sockaddr_in);
	bytes_received=recvfrom(server->sock, (void *)buffer,MAX_MSG_LENGTH, 0, (struct sockaddr *)&(server->sa), &len);
	if(bytes_received==-1){
        bytes_received=errno;
		err_msg("Error receiving message: %s",strerror(errno));
	}
	//PAIN!!!
	else
		buffer[bytes_received]='\0';
	return bytes_received;
}

int TimeoutReceive(server_data *server, char*buffer, struct timeval *t){
	fd_set socks;

	//struct timeval t = {0};

	FD_ZERO(&socks);
	FD_SET(server->sock, &socks);
	//t.tv_sec=timesec;
	if(t->tv_sec==0 && t->tv_usec==0)
		t=NULL;
	/* select function let to wait a finite time for a message */
	if(select(server->sock+1,&socks,NULL,NULL,t)){
		return Receive(server, buffer);
	}
	else {
		//printf("Receiving message timed out!\n");
		return -1;
	}

	return 0;

}

int destroyServer(server_data *server){
	if(close(server->sock)==-1)
		return -1;
	free(server);
	return 0;
}
