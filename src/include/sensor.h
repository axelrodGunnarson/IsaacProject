/* SENSOR.H
 * Purpose of this module is to read each message from the server and parse it, 
 * updating the blackboard whenever necessary. It has been studied in order to 
 * be a separated thread from main process. */

#ifndef SENSOR_H
#define SENSOR_H


#include "libserver.h"
#include "blackboard.h"


#define PLAY_TIMES 18

#define BEFORE_KICK_OFF 0
#define PLAY_ON 1
#define TIME_OVER 2
#define KICK_OFF_L 3
#define KICK_OFF_R 4
#define KICK_IN_L 5
#define KICK_IN_R 6
#define FREE_KICK_L 7
#define FREE_KICK_R 8
#define CORNER_KICK_L 9
#define CORNER_KICK_R 10
#define GOAL_KICK_L 11
#define GOAL_KICK_R 12
#define GOAL_L 13
#define GOAL_R 14
#define DROP_BALL 15
#define OFFSIDE_L 16
#define OFFSIDE_R 17


typedef union obj {
	rplayer_data player;
	rball_data ball;
	rflag_data flag;
} obj;



//parse each received message 
int sensor(server_data *server);

//parse a message
int parse(char *mex);

//skip spaces to the next word, after this function buffer[0] will be the first character different from ' '
int skipSpaces(char *buffer);

//get the next interesting token, parenthesis, number or word (without spaces )
int getToken(char *buffer, char *token);

//parse a Hear message
int parseHear(char *mex);

int translatePlaymode(char *playmode);

//parse a See message
int parseSee(char *mex);

//parse a seen object in a see message
int parseSeeObj(char *mex, int time);

//parse a seen object name, in a see message
int parseSeeObjName(char *mex, obj *tempobj);

//parse an init message
int parseInit(char *mex);

//èarse a reconnect message
int parseReconnect(char *mex);


#endif