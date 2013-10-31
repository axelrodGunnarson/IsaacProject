// TODO
// - ricordinare il codice suddividendo in sottofunzioni
// - curare la sincronizzazione tramite messaggi di hear (eg. prima di far partire il playon aspetto che tutti gli agent mi dicano che sono pronti)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "../include/libserver.h"
#include "../include/errlib.h"
#include "../include/rules.h"

#define APP_NAME "ittrainer"
#define TEAMNAME "Isaac"
#define USAGE_ERR err_quit("Usage: %s [ip] [-pport]",APP_NAME);
#define BADIP_ERR err_quit("Bad IP Address");
#define BADPORT_ERR err_quit("Bad Port");

#define AGENT_NUMBER 1
#define TRAINER_DIR "/home/erix/Dropbox/Manhattan/Luca.Giuliani/"
#define RULE_DIR "rules/"
#define RESULTS_DIR "results/"
#define RESULTS_FILE "res.txt"

#define POPULATION 5

void simulate(char *ip_addr,unsigned int port);
int evaluateSession();
void waitConnections(server_data *server);
void nextGeneration();
int createRulefile(char *dest);


int main(int argc, char * argv[]) {

	int i,j;
	unsigned int ip[4], port=-1;
	char port_string[6], port_string2[6], ip_addr[15]="",temp_string[128];
	int haveport=0, haveip=0, this_ok=0;

	err_init();

	//Check of the arguments
	if(argc > 3){
		USAGE_ERR
	}


	for(i=1;i<argc;i++){
		this_ok=0;

		//avoid buffer overflow!!!
		strncpy(temp_string,argv[i],sizeof(temp_string));
		temp_string[sizeof(temp_string)-1]='\0';

		//check ip definition
		if(sscanf(temp_string,"%u.%u.%u.%u",&ip[0],&ip[1],&ip[2],&ip[3])==4){

			if(haveip){
				USAGE_ERR
			}

			for(j=0;j<4;j++)
				if(!(ip[j]>=0 && ip[j]<=255)){
					BADIP_ERR
				}

			sprintf(ip_addr,"%u.%u.%u.%u",ip[0],ip[1],ip[2],ip[3]);

			if(strcmp(ip_addr,temp_string)!=0){
				USAGE_ERR
			} else{
				haveip++;
				this_ok++;
			}
		}


		//check port definition
		if(strncmp(temp_string,"-p",2)==0){

			if(haveport){
				USAGE_ERR
			}

			if(strlen(temp_string)<3 || strlen(temp_string) > 8){
				USAGE_ERR
			}

			strncpy(port_string,temp_string+2,6);
			port_string[5]='\0';

			sscanf(port_string,"%i",&port);
			if(port<1 || port >65535){
				BADPORT_ERR
				port=-1;
			}

			sprintf(port_string2,"%i",port);
			if(strcmp(port_string,port_string2)!=0){
				USAGE_ERR
			}
			haveport++;
			this_ok++;

		}

		if(this_ok!=1){
			USAGE_ERR
		}

	}


	/* Setting default values if needed */
	if(strcmp(ip_addr, "")==0)
		strcpy(ip_addr,"127.0.0.1");
	if(port==-1)
		port=TRAINER_PORT;

	err_msg("###Debug informations###");
	err_msg("IP: %s",ip_addr);
	err_msg("Port: %u\n",port);



	/* Everything is regular... Let's go!!! */
	err_msg("--- IsaacTeam 2D Soccer Simulation Trainer ---\n");

	
	//for(;;){
		//Update population
		nextGeneration();
		
		//evaluate new generation
		simulate(ip_addr,port);
//	}

	
	printf("CHIUDO!!!!\n\n\n");
	return 0;
}


void simulate(char *ip_addr,unsigned int port){

	int n;
	char buffer[MAX_MSG_SIZE]={ 0 };
	server_data *server;

	FILE *results;
	int ruleset_res;

	char *agent_args[]={"itagent",(char *)0,(char *)0,(char *)0}; /* [0] -> process name
																	 [1] -> rulefile path
																	 [2] -> optional "-r" to reconnect agents */
   																	
	pid_t pid, itagent_pid[AGENT_NUMBER], server_pid,monitor_pid;
	
	int ruleset=0;
	int session_must_go_on=1;

	struct stat st;
	char path[512];
	
	/* Start Connection */
	
	//instantiate the server
	char *server_args[]={"rcssserver","--server::coach=true",(char *)0};

	pid=fork();
	if(pid==0) {//child
		//system("/usr/local/bin/rcssserver --server::coach=true 2> /tmp/errlog.txt");
		execv("/usr/local/bin/rcssserver",server_args);
		exit(0);
	}
	else // parent ( or error )
		server_pid=pid;

	

	//instantiate the monitor
	char *monitor_args[]={"rcssmonitor",(char *)0};

	pid=fork();
	if(pid==0) //child
		execv("/usr/local/bin/rcssmonitor",monitor_args);
	else //parent ( or error )
		monitor_pid=pid;

	sleep(1);
	server=createServer(ip_addr, port);
	tryConnection(server);

	//initialize connection
	strcpy(buffer,"(init)");
	sendMsg(server,buffer);	

	printf("Inizializzazione: ");
	if(receiveMsg(server,buffer)!=-1)
		printf("%s\n",buffer);
	else{
		printf("Inizializzazione fallita\n");
		exit(1);
	}

	strcpy(buffer,"(ear on)");
	sendMsg(server,buffer);	

	printf("Ear: ");
	if(receiveMsg(server,buffer)!=-1)
		printf("%s\n",buffer);
	else{
		printf("ear off!!\n");
		exit(1);
	}

// create result file
	strcpy(path,TRAINER_DIR);
	strcat(path,RESULTS_DIR);
	strcat(path,RESULTS_FILE);

	if(stat(path,&st)==0){
		//printf("Elimino il vecchio file dei risultati\n");
		remove(path);
	}

	results=fopen(path,"a");
	

	//Now start the simulation sessions: a session for each directory in rules/
	

	while(session_must_go_on==1){
		
		char tmp[4];
		strcpy(path,TRAINER_DIR);
		strcat(path,RULE_DIR);
		sprintf(tmp,"%i",ruleset);
		strcat(path,tmp);
		//printf("checking ruleset in: %s\n",path);
		if (stat(path,&st)==0){

			//Evaluate a ruleset of the population
			printf("Evaluating ruleset %i.. \n",ruleset);
						
			for(n=0;n<AGENT_NUMBER;n++){
				//printf("Creo agent %i\n",n);
				char prova[128]="-f";
				char num[10];
				strcat(prova,TRAINER_DIR);
				strcat(prova,RULE_DIR);
				sprintf(tmp,"%i",ruleset);
				strcat(prova,tmp);
				sprintf(tmp,"/rule%i.ita",n);
				strcat(prova,tmp);
				agent_args[1]=prova;
				if(ruleset>0){
					sprintf(num,"-r%i",n+1);
					agent_args[2]=num; //if it is not the first simulation reconnect players!!
				}
				//printf("Rule: %s\n",agent_args[1]);
				pid=fork();
				if(pid==0){ //child
					execv("/usr/local/bin/itagent",agent_args);
					err_msg("Execve fails!!");
				}else{
					waitConnections(server);
					itagent_pid[n]=pid;
				}
			}
			
			/*strncpy(buffer,"(change_mode before_kick_off)",MAX_MSG_SIZE);
			sendMsg(server, buffer);

			struct timeval t;
			t.tv_sec=1;
			t.tv_usec=0;
			printf("attesa di un messaggio\n");
			if(receiveTMsg(server,buffer,&t)!=-1)
				printf("Messaggio ricevuto: %s\n",buffer);
			else
				printf("Ricezione fallita\n");*/

		
			//waitConnections(server);
			//sleep(5);
		
	
			strncpy(buffer,"(change_mode play_on)",MAX_MSG_SIZE);
			sendMsg(server, buffer);
			if(receiveMsg(server,buffer)!=-1)
				printf("Play on: %s\n",buffer);
			else
				printf("Ricezione fallita: play on\n");

			sleep(60);
			strncpy(buffer,"(change_mode before_kick_off)",MAX_MSG_SIZE);
			sendMsg(server, buffer);
			if(receiveMsg(server,buffer)!=-1)
				printf("Before kick off: %s\n",buffer);
			else
				printf("Ricezione fallita: before kick off\n");

			
			for(n=0;n<AGENT_NUMBER;n++){
				kill(itagent_pid[n],SIGKILL);
			}
			for(n=0;n<AGENT_NUMBER;n++){
				waitpid(itagent_pid[n],NULL,0);
			}

			ruleset++;
			
				
			ruleset_res=evaluateSession();
			fprintf(results,"%i\n",ruleset_res);
			printf(".. DONE! Result: %i\n",ruleset_res);


		}else
			session_must_go_on=0;

	}
	
	printf("Evaluation complete!\n");
	
	fclose(results);
	
	kill(server_pid,SIGKILL);
	kill(monitor_pid,SIGKILL);
	waitpid(server_pid,NULL,0);
	waitpid(monitor_pid,NULL,0);
	

	closeConnection(server);
}


int evaluateSession(){
	return rand()%100;
}

void waitConnections(server_data *server){
	char buffer[MAX_MSG_LENGTH];
	int i;
	
	//for(i=0;i<AGENT_NUMBER;i++){
		printf("attesa di un messaggio\n");
		receiveMsg(server,buffer);
		if(strstr(buffer,"ready")!=NULL)
			printf("Messaggio ricevuto: %s\nPlayer ready\n",buffer);
		else
			//i--;
			printf("Messaggio ricevuto: %s, ma non considerato\n",buffer);
	//}
}


void nextGeneration(){
	
	int rule_i,pop_i;

	char path[512]="/tmp/";
	char buf[128];
	mode_t mode=S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH; // new directories have 755 permissions

	struct stat st;


	//clean existing rules dir, if presents
	strcpy(path,"rm -rf ");
	strcat(path,TRAINER_DIR);
	strcat(path,RULE_DIR);
	strcat(path,"*");
	system(path);

	//start creating new generation
	for(pop_i=0;pop_i<POPULATION;pop_i++){	
		for(rule_i=0;rule_i<AGENT_NUMBER;rule_i++){
			strcpy(path,TRAINER_DIR);
			strcat(path,RULE_DIR);
			sprintf(buf,"%i/",pop_i);
			strcat(path,buf);

			//check if ruledir exists, if not create it
			if( stat(path,&st) != 0 )
				if( mkdir(path,mode) != 0)
					err_quit("Unable to create directory: %s",path);
			sprintf(buf,"rule%i.ita",rule_i);
			strcat(path,buf);
			createRulefile(path);		
		}
	}

}

int createRulefile(char *dest){
	
	rules rset;
	int i,j,tmp;

	FILE *out;


	////////////// CREATE A RANDOM RULEFILE ////////////////////
	
	initRules(&rset);

	for(i=0;i<N_RULES;i++){
		//set random conditions
		for(j=0;j<N_CONDITIONS;j++)
			rset.action[PLAY_ON][i].conditions[j]=rand()%2;

		//set random action
		rset.action[PLAY_ON][i].action=rand()%5;
		if(rset.action[PLAY_ON][i].action==1 || rset.action[PLAY_ON][i].action==2){ //if action is GOTO a player, choose the player!
			tmp=(rand()%11)+1;
			if(tmp<10)
				rset.action[PLAY_ON][i].action=(rset.action[PLAY_ON][i].action*10)+tmp;
			else
				rset.action[PLAY_ON][i].action=(rset.action[PLAY_ON][i].action*100)+tmp;
		}
		//abilitate rule
		rset.action[PLAY_ON][i].valid=1;

	}
	//printRules(&rset);	
	out=fopen(dest,"w");
	if(out==NULL)
		err_quit("Can't open dest file: %s",dest);
	fprintRules(&rset,out);
	//printf("Rule saved in %s\n",dest);
	fclose(out);

	////////////// END CREATION /////////////////
	
	return 0;


}

