#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "../include/sensor.h"
#include "../include/rules.h"
#include "../include/errlib.h"
#include "../include/blackboard.h"
#include "../include/libserver.h"


#define STRING_ERR 	err_msg("String format wrong");
#define MAX_TOKENLENGTH 512

//extern sem_t s_seeParsed;


int sensor(server_data *server){
	char buffer[MAX_MSG_SIZE+1]={ 0 };
	//struct timeval t;
	//int time;

	//t.tv_sec=0;
	//t.tv_usec=50000; // AKA 10ms

//	err_msg("Sensor started!!");
	while(1){
		//printf("try to receive msg without timeout\n");
		receiveMsg(server, buffer);
		//time=getTime(&blackboard);
		//printf("%i - received msg\n",time);
		parse(buffer);
		
		/*	
		//try to wait for other msg for 10ms each
		while(receiveTMsg(server,buffer,&t)>0){
			time=getTime(&blackboard);
			//printf("%i - received TIMEOUT msg\n",time);
			parse(buffer);
		}
		*/
		
		
		//sem_post(&s_seeParsed);

	}
	return 0;
}







int parse(char *mex){
	char token[MAX_TOKENLENGTH+1];
	int time;

	//err_msg(mex);

	int get=0;
	getToken(mex, token);
	if(strcmp(token, "(")!=0)
		err_msg("Expected open parenthesis at begin of the server message");
	getToken(mex, token);
	if(strcmp(token,"see")==0){
		parseSee(mex);
		
		get++;
	}
	if(strcmp(token,"init")==0){
		parseInit(mex);
		get++;
	}
	if(strcmp(token,"reconnect")==0){
		parseReconnect(mex);
		get++;
	}
	if(strcmp(token,"sense_body")==0){
		getToken(mex,token);

		if(sscanf(token,"%i",&time)!=1){
			err_msg("Bad time in sense string: %s\n",mex);
		}
		//update time in blackboard;
		setTime(&blackboard, time);
		//printf("\n%i - parse() - New step!\n",time);
		get++;
	}
	if(strcmp(token,"error")==0){
		getToken(mex,token);
		err_msg("Error: %s",token);
		get++;
	}
	if(strcmp(token,"server_param")==0){
		//err_msg("server_param messages not handled!");
		get++;
	}
	if(strcmp(token,"player_param")==0){
		//err_msg("player_param messages not handled!");
		get++;
	}
	if(strcmp(token,"player_type")==0){
		//err_msg("player_type messages not handled!");
		get++;
	}
	if(strcmp(token,"score")==0){
		//err_msg("score messages not handled!");
		get++;
	}
	if(strcmp(token,"hear")==0){
		parseHear(mex);
		get++;
	}
	if(strcmp(token,"msg")==0){
		//parseHear(mex);
		get++;
	}
	if(strcmp(token,"playmode")==0){
		//parseHear(mex);
		get++;
	}
	if(strcmp(token,"team")==0){
		//parseHear(mex);
		get++;
	}
	if(!get){
	    get=getTime(&blackboard);
		err_msg("%i - Server message not identified: (%s %s",get,token, mex);
	}
	return 0;
}


int parseHear(char *mex){
	char token[MAX_TOKENLENGTH+1];
	char sender[MAX_TOKENLENGTH+1];
	int time;
	int playmode=-1;


	//get time
	getToken(mex, token);
	if(sscanf(token,"%i",&time)!=1){
		err_msg("Bad time in hear string: %s\n",mex);
	}

	//get sender
	getToken(mex, token);
	strncpy(sender,token,MAX_TOKENLENGTH);

	if(strcmp(sender,"referee")==0){
		getToken(mex, token);

		//DEBUG
//		err_msg("Referee: %s",token);

		playmode=translatePlaymode(token);
		if(playmode!=-1)
			updatePLaymode(&blackboard, playmode);
		else
			err_msg("Referee: %s",token);

	}
	return 0;

}


int translatePlaymode(char *token){
	if(strcmp(token,"before_kick_off")==0)
		return BEFORE_KICK_OFF;
	if(strcmp(token,"play_on")==0)
		return PLAY_ON;
	if(strcmp(token,"time_over")==0)
		return TIME_OVER;
	if(strcmp(token,"kick_off_l")==0)
		return KICK_OFF_L;
	if(strcmp(token,"kick_off_r")==0)
		return KICK_OFF_R;
	if(strcmp(token,"kick_in_l")==0)
		return KICK_IN_L;
	if(strcmp(token,"kick_in_r")==0)
		return KICK_IN_R;
	if(strcmp(token,"free_kick_l")==0)
		return FREE_KICK_L;
	if(strcmp(token,"free_kick_r")==0)
		return FREE_KICK_R;
	if(strcmp(token,"corner_kick_l")==0)
		return CORNER_KICK_L;
	if(strcmp(token,"corner_kick_r")==0)
		return CORNER_KICK_R;
	if(strcmp(token,"goal_kick_l")==0)
		return GOAL_KICK_L;
	if(strcmp(token,"goal_kick_r")==0)
		return GOAL_KICK_R;
	if(strcmp(token,"goal_l")==0)
		return GOAL_L;
	if(strcmp(token,"goal_r")==0)
		return GOAL_R;
	if(strcmp(token,"drop_ball")==0)
		return DROP_BALL;
	if(strcmp(token,"offside_l")==0)
		return OFFSIDE_L;
	if(strcmp(token,"offside_r")==0)
		return OFFSIDE_R;

	return -1;
}

int parseSee(char *mex){
	char token[MAX_TOKENLENGTH+1];
	int time, objnum=0;

	//get time
	getToken(mex, token);
	if(sscanf(token,"%i",&time)!=1){
		err_msg("Bad time in see string: %s\n",mex);
	}

	//DEBUG
	//printf("%i - parseSee() - See received\n",time);

	//get seen objects
	while(parseSeeObj(mex,time)!=0)
		objnum++;
	if(objnum==0){
		//See message is empty!!!!
		//err_msg("No Object Name in see string: %s\n",mex);
	}

	//get last parenthesis
	getToken(mex,token);
	if(token[0]!=')')
		err_msg("Expected closed parenthesis at the end of see message");

	//sem_post(&s_seeParsed);

	return 0;
}







int parseSeeObj(char *mex, int time){
	char token[MAX_TOKENLENGTH+1]={0};
	obj tempobj;
	int type=0;
	int finished=0;
	float distance=-1;
	float direction=-1;
	float dist_change=-1;
	float dir_change=-1;
	float head_face_dir=-1;
	float body_face_dir=-1;

    memset(&tempobj,0,sizeof(tempobj));

	skipSpaces(mex);
	if(mex[0]!='('){
		return 0; //the string desn't contain seen objects
	}
	getToken(mex, token); //skip the parenthesis
	type=parseSeeObjName(mex, &tempobj); //get the id of te obj ( see blackboard.h ) and if it is a player write team and number, if seen

	//get distance
	getToken(mex, token);
	if(token[0]!=')')
		sscanf(token,"%f",&distance);
	else{
		err_msg("Expected distance not found parsing OBJECT %i",type);
		return 1;
	}

	//get direction
	getToken(mex, token);
	if(token[0]!=')')
		sscanf(token,"%f",&direction);
	else{
		err_msg("Expected direction not found parsing DIRECTION %i",type);
		return 1;
		//finished=1;
	}


	//try get dist_change
	getToken(mex, token);
	if(token[0]!=')')
		sscanf(token,"%f",&dist_change);
	else
		finished=1;


	//try get dir_change
	if(!finished)
		getToken(mex, token);
	if(token[0]!=')')
		sscanf(token,"%f",&dir_change);
	else
		finished=1;


	//try get body_face_dir
	if(!finished)
		getToken(mex, token);
	if(token[0]!=')'){
		sscanf(token,"%f",&body_face_dir);
	} else
		finished=1;


	//try get head_face_dir
	if(!finished)
		getToken(mex, token);
	if(token[0]!=')'){
		sscanf(token,"%f",&head_face_dir);
	} else
		finished=1;


	//get last closed parenthesis or give an error
	if(!finished)
		getToken(mex, token);
	if(token[0]!=')'){
		err_msg("Too much parameters found parsing OBJECT %i",type);

		//skip all parameters until closed parenthesis
		while (token[0]!= ')')
			getToken(mex, token);
		return 1;
	}


	//Assign parsed values to the right type of data in tempobj and then update blackboard
	if(type==-1){
		err_msg("Parsed ObjName not recognized!!!");
	}else if(type>=FLAG_T_L_50 && type <= FLAG_B_R_50 ){

		//update flag data
		tempobj.flag.time=time;
		if(direction!=-1)
			tempobj.flag.direction=direction;
		if(distance!=-1)
			tempobj.flag.distance=distance;
		if(dist_change!=-1)
			tempobj.flag.dist_change=dist_change;
		if(dir_change!=-1)
			tempobj.flag.dir_change=dir_change;

		//udpate blackboard
		updateRelFlag(type, &blackboard, tempobj.flag);
	}else if(type==PLAYER || type == UP){

		//update player data
		tempobj.player.time=time;
		if(direction!=-1)
			tempobj.player.direction=direction;
		if(distance!=-1)
			tempobj.player.distance=distance;
		if(dist_change!=-1)
			tempobj.player.dist_change=dist_change;
		if(dir_change!=-1)
			tempobj.player.dir_change=dir_change;
		if(body_face_dir!=-1)
			tempobj.player.body_face_dir=body_face_dir;
		if(head_face_dir!=-1)
			tempobj.player.head_face_dir=head_face_dir;

		//udpate blackboard
		updateRelPlayer(&blackboard, tempobj.player);
	}else if(type==BALL || type== UB ){

		//update ball data
		tempobj.ball.time=time;
		if(direction!=-1)
			tempobj.ball.direction=direction;
		if(distance!=-1)
			tempobj.ball.distance=distance;
		if(dist_change!=-1)
			tempobj.ball.dist_change=dist_change;
		if(dir_change!=-1)
			tempobj.ball.dir_change=dir_change;

		//udpate blackboard
		updateRelBall( &blackboard, tempobj.ball);
		//printf("%i - seen BALL\n",time);
	}else if (type==UF || type == UG){

		//undefined flag near player... do nothing at now ( usefull? )

	}else if (type >= LINE_L && type <= LINE_B){

		//LINE near player... do nothing at now ( it is useless )

	}
	if(type==-1)
		return -1;

	return 1;
}









int parseSeeObjName(char *mex, obj *tempobj){
	char token[MAX_TOKENLENGTH+1];
	int type;
	int error=0;

	getToken(mex, token);
	if(token[0]!='('){
		err_msg("Bad ObjName, starting with: %s instead of '('\n",token);
		return -1;
	}

	//printf("Parsing object name: %s\n",mex);

	getToken(mex, token);

	switch (token[0]){
		case 'B':;
		case 'b':
			//BALL
			getToken(mex,token);
			if(token[0]==')')
				type= BALL;
			else
				error++;
			break;
		case 'g':
			//GOAL FLAG
			getToken(mex, token);
			if (token[0]=='l')
				type=FLAG_G_L;
			else if (token[0]=='r')
				type=FLAG_G_R;
			else
				error++;
			getToken(mex, token);
			if(token[0]!=')')
				error++;
			break;
		case 'f':
			//GENERIC FLAG: can be followed by
			getToken(mex, token);
			switch (token[0]){


				case 'l':
					// LEFT FLAG: can be followed by b|t|0
					getToken(mex, token);
					switch (token[0]){
						case '0':
							getToken(mex, token);
							if(token[0]==')')
								type=FLAG_L_0;
							else
								error++;
							break;
						case 'b':
							// LEFT FLAG BOTTOM: can be followed by )|10|20|30
							getToken(mex, token);
							if(strncmp(token,")",1)==0)
								type=FLAG_L_B;
							else if(strncmp(token,"10",2)==0){
								getToken(mex, token);
								if(token[0]==')')
									type=FLAG_L_B_10;
								else
									error++;
							} else if(strncmp(token,"20",2)==0){
								getToken(mex, token);
								if(token[0]==')')
									type=FLAG_L_B_20;
								else
									error++;
							} else if(strncmp(token,"30",2)==0){
								getToken(mex, token);
								if(token[0]==')')
									type=FLAG_L_B_30;
								else
									error++;
							} else
								error++;
							break;
						case 't':
							//LEFT FLAG TOP: can be followed by )|10|20|30
							getToken(mex, token);
							if(strncmp(token,")",1)==0)
								type=FLAG_L_T;
							else if(strncmp(token,"10",2)==0){
								getToken(mex, token);
								if(token[0]==')')
									type=FLAG_L_T_10;
								else
									error++;
							} else if(strncmp(token,"20",2)==0){
								getToken(mex, token);
								if(token[0]==')')
									type=FLAG_L_T_20;
								else
									error++;
							} else if(strncmp(token,"30",2)==0){
								getToken(mex, token);
								if(token[0]==')')
									type=FLAG_L_T_30;
								else
									error++;
							} else
								error++;
							break;
					}
					break;


				case 'r':
					//RIGHT FLAG: can be followed by b|t|0
					getToken(mex, token);
					switch (token[0]){
						case '0':
							getToken(mex, token);
							if(token[0]==')')
								type=FLAG_R_0;
							else
								error++;
							break;
						case 'b':
							//RIGHT FLAG BOTTOM: can be followed by )|10|20|30
							getToken(mex, token);
							if(strncmp(token,")",1)==0)
								type=FLAG_R_B;
							else if(strncmp(token,"10",2)==0){
								getToken(mex, token);
								if(token[0]==')')
									type=FLAG_R_B_10;
								else
									error++;
							} else if(strncmp(token,"20",2)==0){
								getToken(mex, token);
								if(token[0]==')')
									type=FLAG_R_B_20;
								else
									error++;
							} else if(strncmp(token,"30",2)==0){
								getToken(mex, token);
								if(token[0]==')')
									type=FLAG_R_B_30;
								else
									error++;
							} else
								error++;
							break;
						case 't':
							//RIGHT FLAG TOP: can be followed by )|10|20|30
							getToken(mex, token);
							if(strncmp(token,")",1)==0)
								type=FLAG_R_T;
							else if(strncmp(token,"10",2)==0){
								getToken(mex, token);
								if(token[0]==')')
									type=FLAG_R_T_10;
								else
									error++;
							} else if(strncmp(token,"20",2)==0){
								getToken(mex, token);
								if(token[0]==')')
									type=FLAG_R_T_20;
								else
									error++;
							} else if(strncmp(token,"30",2)==0){
								getToken(mex, token);
								if(token[0]==')')
									type=FLAG_R_T_30;
								else
									error++;
							} else
								error++;
							break;
						default:
							error++;
					}
					break;
				case 'c':
					//CENTRAL FLAG: can be followed by )|t|b
					getToken(mex, token);
					switch(token[0]){
						case ')':
							type=FLAG_C;
							break;
						case 't':
							getToken(mex, token);
							if(token[0]==')')
								type=FLAG_C_T;
							else
								error++;
							break;
						case 'b':
							getToken(mex, token);
							if(token[0]==')')
								type=FLAG_C_B;
							else
								error++;
							break;
						default:
							error++;
					}
					break;
				case 'p':
					// PORT FLAG: can be followed by l|r
					getToken(mex,token);
					switch(token[0]){
						case 'l':
							getToken(mex, token);
							switch(token[0]){
								case 't':
									getToken(mex, token);
									if (token[0]==')')
										type=FLAG_P_L_T;
									else
										error++;
									break;
								case 'c':
									getToken(mex, token);
									if (token[0]==')')
										type=FLAG_P_L_C;
									else
										error++;
									break;
								case 'b':
									getToken(mex, token);
									if (token[0]==')')
										type=FLAG_P_L_B;
									else
										error++;
									break;
								default:
									error++;
							}
							break;
						case 'r':
							getToken(mex, token);
							switch(token[0]){
								case 't':
									getToken(mex, token);
									if (token[0]==')')
										type=FLAG_P_R_T;
									else
										error++;
									break;
								case 'c':
									getToken(mex, token);
									if (token[0]==')')
										type=FLAG_P_R_C;
									else
										error++;
									break;
								case 'b':
									getToken(mex, token);
									if (token[0]==')')
										type=FLAG_P_R_B;
									else
										error++;
									break;
								default:
									error++;
							}
							break;
						default:
							error++;
					}

					break;
				case 't':
					//TOP FLAG: can be followed by l|r|0
					getToken(mex, token);
					switch(token[0]){
						case 'l':
							//TOP LEFT FLAG: can be followed by 10|20|30|40|50
							getToken(mex, token);
							if(strncmp(token,"10",2)==0)
								type=FLAG_T_L_10;
							else if(strncmp(token,"20",2)==0)
								type=FLAG_T_L_20;
							else if(strncmp(token,"30",2)==0)
								type=FLAG_T_L_30;
							else if(strncmp(token,"40",2)==0)
								type=FLAG_T_L_40;
							else if(strncmp(token,"50",2)==0)
								type=FLAG_T_L_50;
							else
								error++;
							break;
						case 'r':
							//TOP RIGHT FLAG: can be followed by 10|20|30|40|50
							getToken(mex, token);
							if(strncmp(token,"10",2)==0)
								type=FLAG_T_R_10;
							else if(strncmp(token,"20",2)==0)
								type=FLAG_T_R_20;
							else if(strncmp(token,"30",2)==0)
								type=FLAG_T_R_30;
							else if(strncmp(token,"40",2)==0)
								type=FLAG_T_R_40;
							else if(strncmp(token,"50",2)==0)
								type=FLAG_T_R_50;
							else
								error++;
							break;
						case '0':
							getToken(mex, token);
							if(token[0]==')')
								type=FLAG_T_0;
							else
								error++;
							break;
						default:
							error++;
					}
					break;
				case 'b':
					//BOTTOM FLAG: can be followed by l|r|0
					getToken(mex, token);
					switch(token[0]){
						case 'l':
							//BOTTOM LEFT FLAG: can be followed by 10|20|30|40|50
							getToken(mex, token);
							if(strncmp(token,"10",2)==0)
								type=FLAG_B_L_10;
							else if(strncmp(token,"20",2)==0)
								type=FLAG_B_L_20;
							else if(strncmp(token,"30",2)==0)
								type=FLAG_B_L_30;
							else if(strncmp(token,"40",2)==0)
								type=FLAG_B_L_40;
							else if(strncmp(token,"50",2)==0)
								type=FLAG_B_L_50;
							else
								error++;
							break;
						case 'r':
							//BOTTOM RIGHT FLAG: can be followed by 10|20|30|40|50
							getToken(mex, token);
							if(strncmp(token,"10",2)==0)
								type=FLAG_B_R_10;
							else if(strncmp(token,"20",2)==0)
								type=FLAG_B_R_20;
							else if(strncmp(token,"30",2)==0)
								type=FLAG_B_R_30;
							else if(strncmp(token,"40",2)==0)
								type=FLAG_B_R_40;
							else if(strncmp(token,"50",2)==0)
								type=FLAG_B_R_50;
							else
								error++;
							break;
						case '0':
							getToken(mex, token);
							if(token[0]==')')
								type=FLAG_B_0;
							else
								error++;
							break;
						default:
							error++;
					}
					break;

				case 'g':
					//GOAL FLAG: can be followed by l|r
					getToken(mex,token);
					switch(token[0]){
						case 'l':
							getToken(mex, token);
							switch(token[0]){
								case 't':
									getToken(mex, token);
									if (token[0]==')')
										type=FLAG_G_L_T;
									else
										error++;
									break;
								case 'b':
									getToken(mex, token);
									if (token[0]==')')
										type=FLAG_G_L_B;
									else
										error++;
									break;
								default:
									error++;
							}
							break;
						case 'r':
							getToken(mex, token);
							switch(token[0]){
								case 't':
									getToken(mex, token);
									if (token[0]==')')
										type=FLAG_G_R_T;
									else
										error++;
									break;
								case 'b':
									getToken(mex, token);
									if (token[0]==')')
										type=FLAG_G_R_B;
									else
										error++;
									break;
								default:
									error++;
							}
							break;
						default:
							error++;
					}
					break;
				default:
					error++;
			}
			break;
		case 'P':; // UNIDENTIFIED PLAYER ( should skip to normal player )
		case 'p':
			//PLAYER: can be followed by teamname, then number, then "goalie" string
			type=PLAYER;
			getToken(mex, token);
			if(token[0]==')')
				return type;
			//token is the teamname
			strncpy(tempobj->player.team,token,sizeof(char)*MAX_TEAMNAME_LENGHT);
			getToken(mex, token);
			if(token[0]==')')
				return type;
			//token is the number
			sscanf(token,"%i",&(tempobj->player.number));
			getToken(mex, token);
			if(token[0]==')')
				return type;
			//token is GOALIE, nothing to do at now...
			getToken(mex, token);
			if(token[0]!=')')
				error++;
			break;
		case 'l':
			//LINE: can be followed by l|r|t|b
			getToken(mex, token);
			switch(token[0]){
				case 'l':
					getToken(mex, token);
					if(token[0]!=')')
						error++;
					type=LINE_L;
					break;
				case 'r':
					getToken(mex, token);
					if(token[0]!=')')
						error++;
					type=LINE_R;
					break;
				case 't':
					getToken(mex, token);
					if(token[0]!=')')
						error++;
					type=LINE_T;
					break;
				case 'b':
					getToken(mex, token);
					if(token[0]!=')')
						error++;
					type=LINE_B;
					break;
				default:
					error++;
			}

			break;

		//THIS is a real mess... we have notice about element, but not which one... just A flag, or A player... sigh... T.T

		case 'G': //UNIDENTIFIED goal flag
			type=UG;
			getToken(mex, token);
			//getToken(mex, token);
			if(token[0]!=')')
				error++;
			break;
		case 'F': //UNIDENTIFIED flag
			type=UF;
			getToken(mex, token);
			//getToken(mex, token);
			if(token[0]!=')')
				error++;
			break;
		default:
			error++;
	}

	if(error){
		err_msg("Wrong format in SeeObjName: %s\n",mex);
		return -1;
	}
	skipSpaces(mex);
	return type;
}




int parseInit(char *mex){
	char token[MAX_TOKENLENGTH+1];
	int number;
	int playmode;


	/* get side */
	getToken(mex, token);
	if(token[0]!='l' && token[0]!='r')
		err_msg("Wrong side in init message");
	else
		updateSide(&blackboard, token[0]);

	/* get number */
	getToken(mex, token);
	sscanf(token,"%i",&number);
	if (number < 1 || number > 11 )
		err_msg("Wrong player number in init message");
	else
		updateNumber(&blackboard, number);

	/* get playmode, not checked, because it would be booooring */
	getToken(mex,token);
	playmode=translatePlaymode(token);
	updatePLaymode(&blackboard, playmode);

	/* get last parenthesis */
	getToken(mex, token);
	if(token[0]!=')')
		err_msg("Expected closed parenthesis at the end of init message!!");

	return 0;
}


int parseReconnect(char *mex){
	char token[MAX_TOKENLENGTH+1];
	int playmode;


	/* get side */
	getToken(mex, token);
	if(token[0]!='l' && token[0]!='r')
		err_msg("Wrong side in reconnect message");
	else
		updateSide(&blackboard, token[0]);

	/* get playmode, not checked, because it would be booooring */
	getToken(mex,token);
	playmode=translatePlaymode(token);
	updatePLaymode(&blackboard, playmode);

	/* get last parenthesis */
	getToken(mex, token);
	if(token[0]!=')')
		err_msg("Expected closed parenthesis at the end of reconnect message!!");

	return 0;
}





int skipSpaces(char *buffer){
	int i=0;
	int j;
	while(buffer[i]==' ')
		i++;
	if(i)
		for(j=0;j<strlen(buffer)-i;j++)
			buffer[j]=buffer[j+i];
	return 0;
}



int getToken(char *buffer, char *token){
	int i=0;
	int j;
	char tempbuf[MAX_MSG_SIZE];

	//printf("buffer vale: %s",buffer);
	if(buffer[0]=='\0'){
		return 0;
	}
	skipSpaces(buffer);

	//da rivedere TUTTO
	token[0]=buffer[0];
	if(token[0]=='(' || token[0]==')' ){
		for(j=0;j<strlen(buffer);j++)
			buffer[j]=buffer[j+1];
		token[1]='\0';
	}else{
		while(buffer[i]!='(' && buffer[i]!=')' && buffer[i]!=' '){
			i++;
		}
		// now buffer[i] is the firs character after the interesting token and i is the characters number of the token
		strncpy(token,buffer,i*sizeof(char));
		token[i]='\0';

		//delete token from buffer
		
		buffer[MAX_MSG_SIZE]='\0';
		strncpy(tempbuf,&buffer[i],MAX_MSG_SIZE);
		strncpy(buffer,tempbuf,MAX_MSG_SIZE);

		/*for(j=0;j<strlen(buffer)-i;j++)
			buffer[j]=buffer[j+i];
		buffer[j]='\0';*/
	}

	return 1;
}


