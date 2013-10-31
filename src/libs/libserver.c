#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/libserver.h"
#include "../include/errlib.h"
#include "../include/libsocket.h"


sem_t conn;

int tryConnection(server_data *server){
	sem_init(&conn, 0, 1);
	char buffer[MAX_MSG_LENGTH]="", mex='-';
	sendMsg(server,&mex);

	//err_msg("Provo la connessione");
	/* if not receive answer or receive the same mex back fails */
	if(receiveMsg(server, buffer)!=-1 && buffer[0]!=mex ){
		//err_msg("Connection succeded");
	}else{
		err_msg("Connection failed");
		exit(1);
	}
	return 0;
}



int receiveMsg(server_data *server, char* buffer){
	int state;
	struct timeval t;
	t.tv_sec =0;
	t.tv_usec=0;
	//printf("cerco di bloccare il semaforo per leggere\n");
	sem_wait(&conn);
	//printf("bloccato il semaforo per leggere\n");
	state=TimeoutReceive(server,buffer,&t);
	sem_post(&conn);
	//printf("rilasciato il semaforo dopo aver letto\n");
	return state;
}

int receiveTMsg(server_data *server, char* buffer, struct timeval *t){
	int state;
	struct timeval tv;

	memcpy(&tv,t,sizeof(struct timeval)); // copy the value to avoid touching original variable
	//printf("cerco di bloccare il semaforo per leggere\n");
	sem_wait(&conn);
	//printf("bloccato il semaforo per leggere\n");
	state=TimeoutReceive(server,buffer,&tv);
	sem_post(&conn);
	//printf("rilasciato il semaforo dopo aver letto\n");
	return state;
}

int sendMsg(server_data *server, char* buffer){
	int state;
	//printf("cerco di bloccare il semaforo per scrivere\n");
	sem_wait(&conn);
	//printf("bloccato il semaforo per scrivere\n");
		state= Send(server,buffer);
	sem_post(&conn);
	//printf("rilasciato il semaforo dopo aver scritto\n");
	return state;
}



int closeConnection(server_data *server){

	/* send conclusion token and then destroy */
	sendMsg(server, "(bye)");
	destroyServer(server);
	return 0;
}

