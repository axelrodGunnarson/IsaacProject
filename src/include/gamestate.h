#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <libserver.h>

typedef struct _PlayerData{
	int connected;
	float x;
	float y;
	float direction;
	float dist_change;
	float dir_change;
	float head_face_dir;
	float body_face_dir;

} PlayerData;


typedef struct _TeamData{

	PlayerData players[11];
	char teamname[MAX_TEAMNAME_LENGHT];
	int goals;

} TeamData

typedef struct _SituationData {
	
	TeamData teams[2];


} SituationData



#endif

