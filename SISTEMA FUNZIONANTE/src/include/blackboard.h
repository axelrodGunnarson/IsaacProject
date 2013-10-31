#ifndef BLACKBOARD_H
#define BLACKBOARD_H

/* Blackboard library
 * This document defines all necessaries definitions and prototypes used in order to access to blackboard data */

#include <semaphore.h>
#include "libserver.h"


#define ALOTOF_PLAYERS 512

#define FLAG_T_L_50 0
#define FLAG_T_L_40 1
#define FLAG_T_L_30 2
#define FLAG_T_L_20 3
#define FLAG_T_L_10 4
#define FLAG_T_0 5
#define FLAG_T_R_10 6
#define FLAG_T_R_20 7
#define FLAG_T_R_30 8
#define FLAG_T_R_40 9
#define FLAG_T_R_50 10
#define FLAG_L_T 11
#define FLAG_C_T 12
#define FLAG_R_T 13
#define FLAG_L_T_30 14
#define FLAG_R_T_30 15
#define FLAG_L_T_20 16
#define FLAG_P_L_T 17
#define FLAG_P_R_T 18
#define FLAG_R_T_20 19
#define FLAG_L_T_10 20
#define FLAG_R_T_10 21
#define FLAG_G_L_T 22
#define FLAG_G_R_T 23
#define FLAG_L_0 24
#define FLAG_G_L 25
#define FLAG_P_L_C 26
#define FLAG_C 27
#define FLAG_P_R_C 28
#define FLAG_G_R 29
#define FLAG_R_0 30
#define FLAG_G_L_B 31
#define FLAG_G_R_B 32
#define FLAG_L_B_10 33
#define FLAG_R_B_10 34
#define FLAG_L_B_20 35
#define FLAG_P_L_B 36
#define FLAG_P_R_B 37
#define FLAG_R_B_20 38
#define FLAG_L_B_30 39
#define FLAG_R_B_30 40
#define FLAG_L_B 41
#define FLAG_C_B 42
#define FLAG_R_B 43
#define FLAG_B_L_50 44
#define FLAG_B_L_40 45
#define FLAG_B_L_30 46
#define FLAG_B_L_20 47
#define FLAG_B_L_10 48
#define FLAG_B_0 49
#define FLAG_B_R_10 50
#define FLAG_B_R_20 51
#define FLAG_B_R_30 52
#define FLAG_B_R_40 53
#define FLAG_B_R_50 54
#define PLAYER 55
#define BALL 56
#define UP 57
#define UF 58
#define UG 59
#define UB 60
#define LINE_L 61
#define LINE_R 62
#define LINE_T 63
#define LINE_B 64



// To be Moved!!!!!
typedef enum {FALSE, TRUE} boolean;

typedef struct rFlag_data{
	int time;
	float distance;
	float direction;
	float dist_change;
	float dir_change;

} rflag_data;

typedef struct rPlayer_data{
	int time;
	float distance;
	float direction;
	float dist_change;
	float dir_change;
	float head_face_dir;
	float body_face_dir;
	char team[MAX_TEAMNAME_LENGHT+1];
	int number;

} rplayer_data;

typedef struct rBall_data{
	int time;
	float distance;
	float direction;
	float dist_change;
	float dir_change;

} rball_data;

typedef struct aFlag_data{
	int time;
	float x;
	float y;
	float distchange;
	float dirchange;

} aflag_data;

typedef struct aPlayer_data{
	int time;
	float x;
	float y;
	float direction;
	float dist_change;
	float dir_change;
	float head_face_dir;
	float body_face_dir;
	float radius;
	char team[MAX_TEAMNAME_LENGHT+1];
	int number;

} aplayer_data;


typedef struct aBall_data{
	int time;
	float x;
	float y;
	float direction;
	float dist_change;
	float dir_change;

} aball_data;


typedef struct Envir_absdata{
	aflag_data flags[N_FLAGS];
	aplayer_data players[ALOTOF_PLAYERS];
	int n_players;
	aball_data ball;
	sem_t mutex;
	sem_t r_sem;
	int readers;
} envir_absdata;

typedef struct Envir_reldata{
	rflag_data flags[N_FLAGS];
	rplayer_data players[ALOTOF_PLAYERS];
	int n_players;
	rball_data ball;

	sem_t mutex;
	sem_t r_sem;
	int readers;
} envir_reldata;


typedef struct Body_data{
	int time;
	float x;
	float y;
	double body_angle;
	float stamina;
	float effort;
	float amount_speed;
	float direction_speed;
	float head_angle;
	int kickcount;
	int dashcount;
	int turncount;
	int saycount;
	int turn_neckcount;
	int catchcount;
	int movecount;
	int change_viewcount;
} body_data;

typedef struct Ai_data{
	float dest_x;
	float dest_y;
	float dest_alfa;

} ai_data;

typedef struct Ipotetic_offset{
	float d_x;
	float d_y;
	double d_alfa;

	float precision;
} ipotetic_offset;

typedef struct Body_struct{
	body_data data;
	ipotetic_offset delta;

	sem_t mutex;
	sem_t r_sem;
	int readers;
} body_struct;

typedef struct Ai_struct{
	ai_data data;
	int valid;

	sem_t mutex;
	sem_t r_sem;
	int readers;

} ai_struct;

typedef struct Blackboard_struct{
	envir_absdata absolute;
	envir_reldata relative;
	body_struct body;
	ai_struct ai;
	int number;
	char team[MAX_TEAMNAME_LENGHT+1];
	char side;
	int now;
	int playmode;

	sem_t mutex;
	sem_t r_sem;
	int readers;
} blackboard_struct;

extern blackboard_struct blackboard;


/* blackboard cleaner thread */
void cleaner();

/* initialise blackboard semaphores and values */
int blackboardInit(blackboard_struct *blackboard);

/* update cycle time */
int setTime(blackboard_struct *blackboard, int time);

/* get cycle time */
int getTime(blackboard_struct *blackboard);

/* Obtain data of a flag in the relative vector */
rflag_data getRelFlag(int flag,blackboard_struct *blackboard);

/* Update data of a flag in the relative vector */
void updateRelFlag(int flag, blackboard_struct *blackboard, rflag_data data);

/* Obtain data of a flag in the absolute vector */
aflag_data getAbsFlag(int flag,blackboard_struct *blackboard);

/* Update data of a flag in the absolute vector */
void updateAbsFlag(int flag, blackboard_struct *blackboard, aflag_data data);

/* Copy a relative player structure, using strcmp to clone strings */
void copyRPlayer(rplayer_data *dst, rplayer_data *src);

/* Copy an absolute player structure, using strcmp to clone strings */
void copyAPlayer(aplayer_data *dst, aplayer_data *src);

/* Obtain data of relative players in a compact vector */
void getRelPlayers(blackboard_struct *blackboard, int *num, rplayer_data *tempplayers);

/* Update data of a player in the relative vector: if it is recognized (by team and number)
 * in the vector it is updated, else it is inserted in the first free position */
void updateRelPlayer(blackboard_struct *blackboard, rplayer_data data);

/* Obtain data of absolute players in a compact vector */
void getAbsPlayers(blackboard_struct *blackboard, int *num, aplayer_data *tempplayers);

/* Update data of a player in the absolute vector: if it is recognized (by team and number)
 * in the vector it is updated, else it is inserted in the first free position */
 void updateAbsPlayer(blackboard_struct *blackboard, aplayer_data data);

/* Obtain data of the ball in the relative vector */
rball_data getRelBall(blackboard_struct *blackboard);

/* Update data of the in the relative vector */
void updateRelBall(blackboard_struct *blackboard, rball_data data);

/* Obtain data of the ball in the absolute vector */
aball_data getAbsBall(blackboard_struct *blackboard);

/* Update data of the ball in the absolute vector */
void updateAbsBall(blackboard_struct *blackboard, aball_data data);

/* Obtain data of the body */
body_data getBody(blackboard_struct *blackboard);

/* Update data of the body */
void updateBody(blackboard_struct *blackboard, body_data data);

/* Get the offset calculated on action sent */
ipotetic_offset getOffset(blackboard_struct *blackboard);

/* Add offset for an action sent */
void addOffset(blackboard_struct *blackboard, ipotetic_offset delta);

/* Apply offset changes to body coordinates and angle */
void applyOffset(blackboard_struct *blackboard);

/* Reset the offset and restore max precision */
void resetOffset(blackboard_struct *blackboard);

/* Obtain data of the AI */
ai_data getAi(blackboard_struct *blackboard);

/* Update data of the AI */
void updateAi(blackboard_struct *blackboard, ai_data data);

/* AI says: do nothing!!!  */
void nopAi(blackboard_struct *blackboard);

/* return pointer to a copy of the teamname */
char *GetTeam(blackboard_struct *blackboard, char *team);

/* return agent number */
int GetNumber(blackboard_struct *blackboard);

/* Insert team and number of the agent in the blackboard */
void updateTeam(blackboard_struct *blackboard, char *team);

void updateNumber(blackboard_struct *blackboard, int num);

/*return side char of the team */
char getSide(blackboard_struct *blackboard);

/* put side of the team in the blackboard */
void updateSide(blackboard_struct *blackboard, char side);

/* update playmode */
void updatePLaymode(blackboard_struct *blackboard, int playmode);

/* return playmode */
int getPLaymode(blackboard_struct *blackboard);

/* return if ai decisions are valid */
int aiStatus(blackboard_struct *blackboard);

/* Print all visible relative flags */
int printRelFlags(blackboard_struct *blackboard,char *buffer);

/* Print all visible absolute flags */
int printAbsFlags(blackboard_struct *blackboard,char *buffer);

#endif
