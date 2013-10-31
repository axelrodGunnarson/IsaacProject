/*	TODO
 *
 * - l'agente vede la palla una volta ogni 3 turni.. perchè?????
 *
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <unistd.h>

#include "../include/blackboard.h"
#include "../include/libserver.h"
#include "../include/sensor.h"
#include "../include/errlib.h"
#include "../include/trajectory.h"
#include "../include/localization.h"
#include "../include/sender.h"
#include "../include/ai.h"

#define APP_NAME "itagent"
#define TEAMNAME "Isaac"
#define RULE_FILE "rule.ita"
#define MAX_RULEFILE_NAME_L 512
#define USAGE_ERR err_quit("Usage: %s [ip] [-p<port>] [-t<teamname>] [-g] [-f<rulepath>] [-r]",APP_NAME);
#define BADIP_ERR err_quit("Bad IP Address");
#define BADPORT_ERR err_quit("Bad Port");
#define BADTEAMNAME_ERR err_quit("Teamname too long!");
#define BADFILENAME_ERR err_quit("Filename too long!");



blackboard_struct blackboard;
rules it_rules;
sem_t s_cleaner, s_localization,s_trajectory, s_ai;
sem_t s_localizationComplete,s_seeParsed, s_aiComplete, s_trajectoryComplete, s_cleanerComplete;

void agentClock(){
    clock_t start, end;
	double nsec=0;
	struct timespec cycle;
	cycle.tv_sec=0;
	cycle.tv_nsec=SIMULATION_STEP_MS*(1e6);

	int sem_val;
	int play_mode;
	play_mode=getPLaymode(&blackboard);

	while(1){
		start=clock();
		//err_msg("clock!\n");		
		//getvalue of semaphores is useful to avoid scheduler jokes 
		//like making a module wating for a step and then doing two step together
		sem_getvalue(&s_cleaner,&sem_val);
		if(sem_val==0)
			sem_post(&s_cleaner);
        
		sem_getvalue(&s_localization,&sem_val);
		if(sem_val==0)
			sem_post(&s_localization);
        
		sem_getvalue(&s_trajectory,&sem_val);
		if(sem_val==0)
			sem_post(&s_trajectory);

		sem_getvalue(&s_ai,&sem_val);
		if(sem_val==0)
			sem_post(&s_ai);

		//sem_wait(&s_trajectoryComplete); // wait unitl trajectory has done, to be sure most stuff has been complete



		end=clock();

        nsec=(((double)end-start)/CLOCKS_PER_SEC)*(1e9);
		//printf("Completed in %.2fms\n",nsec/1e6);
        cycle.tv_nsec=SIMULATION_STEP_MS*(1e6)-nsec;
        // decommentare per andare a 1s di step
		//cycle.tv_sec=1;
		//cycle.tv_nsec=1;
		if(cycle.tv_nsec>0)
			nanosleep(&cycle,NULL);
		else if (play_mode == PLAY_ON)
			err_msg("agentClock() - DANGER, time is too fast!");

	}
}





//this function is a test function. It create a shell, if you write an int it gives relative flags data,
//if you write a command it executes it!!
void command(server_data *server){
	char buffer[MAX_MSG_SIZE+1]= { 0 };
	int i=-1;
	int flag=0;
	char tempstr[10000+1];

	rflag_data rflag1={0};
	aflag_data aflag1={0};
	body_data body={0};
	ai_data ai={0};
	rball_data ball={0};
	//test variables

	sleep(1);

	while(1){
		printf(">");
		fflush(stdout);
		i=-1;
		do{
			i++;
			read(0, buffer+i,sizeof(char));
		}while(buffer[i]!='\n');

		buffer[i]='\0';
		if(buffer[0]=='Q')
			exit(0);
		if(buffer[0]=='(')
			sendAction(server, buffer);
		else if(buffer[0]=='B'){
			body=getBody(&blackboard);
			printf("body:\n\tTIME:%i\n\tBTIME=%i.00\n\tx=%f\n\ty=%f\n\talfa=%f\n",getTime(&blackboard),body.time, body.x, body.y, body.body_angle);
		}
		else if(buffer[0]=='b'){
			ball=getRelBall(&blackboard);
			printf("ball:\n\tTIME:%i\n\tBTIME=%i.00\n\tdistance=%f\n\tdirection=%f\n",getTime(&blackboard),ball.time, ball.distance, ball.direction);

		}else if(buffer[0]=='D'){
			sscanf(buffer,"D %f %f %f\n",&ai.dest_x, &ai.dest_y,&ai.dest_alfa);
			updateAi(&blackboard, ai);
			err_msg("inviato comando %f %f %f\n", ai.dest_x, ai.dest_y, ai.dest_alfa);
		}else if(buffer[0]=='R'){
			sscanf(buffer,"R %i",&flag);
			if (flag<=LINE_B){
				rflag1=getRelFlag(flag,&blackboard);
				printf("###Relative Flag number %i\n\n\ttime=%i\n\tdist=%f\n\talfa=%f\n",flag, rflag1.time, rflag1.distance, rflag1.direction);
				fflush(stdout);
			}
		}else if(buffer[0]=='A'){
			sscanf(buffer,"A %i",&flag);
			if (flag<=LINE_B){
				aflag1=getAbsFlag(flag,&blackboard);
				printf("###Absolute Flag number %i\n\n\t%i\n\t%f\n\t%f\n",flag, aflag1.time, aflag1.x, aflag1.y);
				fflush(stdout);
			}
		}else if(buffer[0]=='P'){
			printRelFlags(&blackboard,tempstr);
			err_msg("%s",tempstr);
			fflush(stdout);

		}else if(buffer[0]=='O'){
			applyOffset(&blackboard);
		}
	}
}





int main(int argc, char * argv[]) {
	int i,j/*,n*/;
	int pos_x, pos_y;
	int goalie=0;
	char buffer[MAX_MSG_SIZE]={ 0 };
	unsigned int ip[4], port=-1;
	char teamname[MAX_TEAMNAME_LENGHT+1]="";
	char port_string[6], port_string2[6], ip_addr[15]="",temp_string[128], reconnect_num[10];
	int haveteam=0, haveport=0, haveip=0, havefile=0, this_ok=0, reconnect=0;
	server_data *server;
	pthread_t parser_id, command_id, cleaner_id, trajectory_id, localization_id, /*ai_id, */agentclock_id;
	char rulefile[MAX_RULEFILE_NAME_L]=RULE_FILE;

	err_init();


	//Check of the arguments
	if(argc > 8){
		USAGE_ERR
	}


	for(i=1;i<argc;i++){
		this_ok=0;
		

		//avoid buffer overflow!!!
		strncpy(temp_string,argv[i],sizeof(temp_string));
		temp_string[sizeof(temp_string)-1]='\0';
		
		//check goalie definition
		if(strncmp(temp_string,"-g",2)==0){
			goalie=1;
			this_ok++;
		}
		
		//check reconnect flag
		if(strncmp(temp_string,"-r",2)==0){
			if(reconnect){ //duplicate reconnect definition
				USAGE_ERR
			}
			if(strlen(temp_string)>2){
				char support[10];

				strncpy(support,temp_string+2,9);
				strncpy(reconnect_num,support,9);
				if(strcmp(reconnect_num, temp_string+2)!=0){
					USAGE_ERR
				}
				//printf("Reconnect with num %s\n",reconnect_num);
				reconnect=1;

			}
			this_ok++;
		}

		//check team definition
		if(strncmp(temp_string,"-t",2)==0){

			if(haveteam){ //duplicate team definition
				USAGE_ERR
			}

			if(strlen(temp_string)>2){
				strncpy(teamname,temp_string+2,MAX_TEAMNAME_LENGHT);
				if(strcmp(teamname, temp_string+2)!=0){
					BADTEAMNAME_ERR
				}

				haveteam++;
				this_ok++;
			}else {
				USAGE_ERR
			}
		}
		
		//check rulefile definition
		if(strncmp(temp_string,"-f",2)==0){

			if(havefile){ //duplicate file definition
				USAGE_ERR
			}

			if(strlen(temp_string)>2){
				char support[MAX_RULEFILE_NAME_L];
				strncpy(support,temp_string+2,MAX_RULEFILE_NAME_L);
				strncpy(rulefile,support,MAX_RULEFILE_NAME_L);
				if(strcmp(rulefile, temp_string+2)!=0){
					BADFILENAME_ERR
				}

				havefile++;
				this_ok++;
			}else {
				USAGE_ERR
			}
		}

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
	if(strcmp(teamname,"")==0)
		strncpy(teamname,TEAMNAME,sizeof(teamname));

	if(strcmp(ip_addr, "")==0)
		strcpy(ip_addr,"127.0.0.1");
	if(port==-1)
		port=PORT;
	/*
	err_msg("###Debug informations###");
	err_msg("Team: %s",teamname);
	err_msg("IP: %s",ip_addr);
	err_msg("Port: %u\n",port);
*/


	/* Everything is regular... Let's go!!! */
//	err_msg("--- IsaacTeam 2D Soccer Simulation Agent ---\n");

	/* Rule Parsing */
	/*rfile=fopen(rulefile,"r");
	if(!rfile)
		err_quit("rule file missing: %s",rulefile);*/
		
	initRules(&it_rules);
	//parseRules(&it_rules, rfile);

	//DEBUG
	//printRules(&it_rules);

	/* Start Connection */

	server=createServer(ip_addr, port);
	tryConnection(server);
	/* Server connected, initialising player */
	if(!reconnect)
		strcpy(buffer,"(init ");
	else{
		strcpy(buffer,"(reconnect ");
	}
	strcat(buffer,teamname);
	if(goalie==1) //if goali add the flag in the message
		strcat(buffer," (goalie)");
	
	if(reconnect){ //if you want to reconnect the player
		strcat(buffer, " ");
		strcat(buffer,reconnect_num);
	}
	strcat(buffer,")");
	//printf("messaggio di connessione: %s\n",buffer);
	sendMsg(server, buffer);
	//sendMsg(server,"(say hello)");

	blackboardInit(&blackboard);

	//init clock semaphores
    sem_init(&s_localizationComplete,0,0);
    sem_init(&s_trajectoryComplete,0,0);
    sem_init(&s_cleanerComplete,0,0);
    sem_init(&s_aiComplete,0,0);
    sem_init(&s_seeParsed,0,0);
	sem_init(&s_cleaner,0,0);
   	sem_init(&s_localization,0,0);
   	sem_init(&s_trajectory,0,0);
   	sem_init(&s_ai,0,0);
	//sendAction(server,"(turn_neck 90)");

	/* update team */
	updateTeam(&blackboard,teamname);
	
	/* if reconnected set uniform number */
	if(reconnect)
		updateNumber(&blackboard,atoi(reconnect_num));

	/* starting parser thread */
	if(pthread_create(&parser_id, NULL, (void *)sensor, (void *)server))
		err_quit("Failed to create sensor thread!!");

	/*starting command line thread - Just for Testing Purpose! */
	if(pthread_create(&command_id, NULL, (void *)command, (void *)server))
		err_quit("Failed to create command thread!!");

	/*starting trajectories thread */
	if(pthread_create(&trajectory_id, NULL, (void *)trajectory_thread, (void *)server))
		err_quit("Failed to create trajectories thread!!");

	/*starting localization thread */
	if(pthread_create(&localization_id, NULL, (void *)localization, NULL))
		err_quit("Failed to create localization thread!!");

	/*starting blackboard cleaner thread */
	if(pthread_create(&cleaner_id, NULL, (void *)cleaner, NULL))
		err_quit("Failed to create cleaner thread!!");

	/*starting AI thread */
	/*if(pthread_create(&ai_id, NULL, (void *)aiThread, NULL))
		err_quit("Failed to create AI thread!!");
*/

    /*starting agentClock thread */
	if(pthread_create(&agentclock_id, NULL, (void *)agentClock, NULL))
		err_quit("Failed to create agentClock thread!!");

	//move to initial position CASUAL!!!
	sleep(1);
	int temp=GetNumber(&blackboard);
	//	printf("temp vale %i\n",temp);
	srand(temp);
	//pos_x=-20;
	//pos_y=-30 + temp*6;
	pos_x=-(rand()%20);	
	pos_y=(rand()%60)-30;

	sprintf(buffer,"(move %i %i)",pos_x,pos_y);
	//	printf("%s\n",buffer);
	sendAction(server,buffer);



	printf("Player %i connected!\n",temp);
	sprintf(buffer,"(say Player %i ready)",temp);
	sendMsg(server,"(say ready)");

	/* join parser thread */
	pthread_join(parser_id, NULL);
	pthread_join(command_id, NULL);
	pthread_join(trajectory_id, NULL);
	pthread_join(localization_id, NULL);
	pthread_join(cleaner_id,NULL);
    //pthread_join(ai_id,NULL);
    pthread_join(agentclock_id,NULL);



	closeConnection(server);

	return 0;
}
