#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "../include/blackboard.h"
#include "../include/errlib.h"
#include "../include/libserver.h"

#define NCICLE_OLD 1 //each object older than 1 cycles in blackboard must be deleted

extern sem_t s_cleaner;
extern sem_t s_cleanerComplete;

void cleaner(){
    clock_t start,end;
    double nsec;
	int i, now=0;

	while(1){

		sem_wait(&s_cleaner);
		start=clock();
		//err_msg("cleaner\n");
		now=getTime(&blackboard);

		//DEBUG
		//err_msg("It's Time %i and I clean!!",now);

		//Block semahpores
		sem_wait(&(blackboard.relative.mutex));
		sem_wait(&(blackboard.absolute.mutex));

		//update flags data
		for(i=0;i<N_FLAGS;i++){
			if(blackboard.relative.flags[i].time < (now - NCICLE_OLD))
				blackboard.relative.flags[i].time = -1;
			if(blackboard.absolute.flags[i].time < (now - NCICLE_OLD))
				blackboard.absolute.flags[i].time = -1;
		}

		//update players data
		for(i=0;i<N_PLAYERS;i++){
			if(blackboard.relative.players[i].time < (now - NCICLE_OLD))
				blackboard.relative.players[i].time = -1;
			if(blackboard.absolute.players[i].time < (now - NCICLE_OLD))
				blackboard.absolute.players[i].time = -1;
		}

		//update ball data
		if(blackboard.relative.ball.time < (now - NCICLE_OLD))
			blackboard.relative.ball.time = -1;
		if(blackboard.absolute.ball.time < (now - NCICLE_OLD))
			blackboard.absolute.ball.time = -1;


		//Release  semaphore
		sem_post(&(blackboard.absolute.mutex));
		sem_post(&(blackboard.relative.mutex));

		sem_post(&s_cleanerComplete);

		end=clock();


		nsec=(((double)end-start)/CLOCKS_PER_SEC)*(1e9);
        if(nsec>SIMULATION_STEP_MS*(1e6))
			err_msg("cleaner() - DANGER, time is too fast!");

	}
}



int blackboardInit(blackboard_struct *blackboard){

	if(blackboard == NULL)
		return 1;

	//initialise all the struct with -1, in order to avoid presence of rubbish around ( and have default value of time fields -1! )
	memset(blackboard,-1,sizeof(blackboard_struct));


	//initialise all semaphores
	sem_init(&(blackboard->absolute.mutex),0,1);
	sem_init(&(blackboard->absolute.r_sem),0,1);

	sem_init(&(blackboard->relative.mutex),0,1);
	sem_init(&(blackboard->relative.r_sem),0,1);

	sem_init(&(blackboard->body.mutex),0,1);
	sem_init(&(blackboard->body.r_sem),0,1);

	sem_init(&(blackboard->ai.mutex),0,1);
	sem_init(&(blackboard->ai.r_sem),0,1);

	sem_init(&(blackboard->mutex),0,1);
	sem_init(&(blackboard->r_sem),0,1);

	blackboard->absolute.readers=0;
	blackboard->relative.readers=0;
	blackboard->body.readers=0;
	blackboard->ai.readers=0;
	blackboard->readers=0;

	blackboard->body.data.body_angle=0;
	blackboard->body.data.head_angle=0;

	resetOffset(blackboard);


//	err_msg("Blackboard initialized!");

	return 0;
}


rflag_data getRelFlag(int flag,blackboard_struct *blackboard){

	rflag_data tempflag={-1};

	if(blackboard == NULL)
		err_quit("NULL reference error!!!!");


	tempflag.time=-1;
	if (flag <FLAG_T_L_50 || flag >FLAG_B_R_50 )
		return tempflag;

	//Block semaphores
	sem_wait(&(blackboard->relative.r_sem));
	blackboard->relative.readers++;
	if(blackboard->relative.readers==1)
		sem_wait(&(blackboard->relative.mutex));
	sem_post(&(blackboard->relative.r_sem));

	//Copy data
	tempflag=blackboard->relative.flags[flag];

	//Release semaphores
	sem_wait(&(blackboard->relative.r_sem));
	blackboard->relative.readers--;
	if(blackboard->relative.readers==0)
		sem_post(&(blackboard->relative.mutex));
	sem_post(&(blackboard->relative.r_sem));

	return tempflag;
}

void updateRelFlag(int flag, blackboard_struct *blackboard, rflag_data data){

	if(blackboard == NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->relative.mutex));

	//update data
	blackboard->relative.flags[flag]=data;
	//Release semaphore
	sem_post(&(blackboard->relative.mutex));

	return;
}

aflag_data getAbsFlag(int flag,blackboard_struct *blackboard){
	aflag_data tempflag={-1};

	if(blackboard == NULL)
		err_quit("NULL reference error!!!!");


	if (flag <FLAG_T_L_50 || flag >FLAG_B_R_50 )
		return tempflag;

	//Block semaphores
	sem_wait(&(blackboard->absolute.r_sem));
	blackboard->absolute.readers++;
	if(blackboard->absolute.readers==1)
		sem_wait(&(blackboard->absolute.mutex));
	sem_post(&(blackboard->absolute.r_sem));

	//Copy data
	tempflag=blackboard->absolute.flags[flag];

	//Release semaphores
	sem_wait(&(blackboard->absolute.r_sem));
	blackboard->absolute.readers--;
	if(blackboard->absolute.readers==0)
		sem_post(&(blackboard->absolute.mutex));
	sem_post(&(blackboard->absolute.r_sem));

	return tempflag;
}

void updateAbsFlag(int flag, blackboard_struct *blackboard, aflag_data data){

	if(blackboard == NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->absolute.mutex));

	//update data
	blackboard->absolute.flags[flag]=data;

	//Release sempahore
	sem_post(&(blackboard->absolute.mutex));

	return;
}


void copyRPlayer(rplayer_data *dst, rplayer_data *src){

	if(dst == NULL || src == NULL)
		err_quit("NULL reference error!!!!");

	*dst=*src;
	/* strncpy needed to duplicate team string */
	strncpy(dst->team,src->team,MAX_TEAMNAME_LENGHT);
	dst->team[MAX_TEAMNAME_LENGHT]='\0';

}

void copyAPlayer(aplayer_data *dst, aplayer_data *src){

	if(dst == NULL || src == NULL)
		err_quit("NULL reference error!!!!");

	*dst=*src;
	/* strncpy needed to duplicate team string */
	strncpy(dst->team,src->team,MAX_TEAMNAME_LENGHT);
	dst->team[MAX_TEAMNAME_LENGHT]='\0';

}


void getRelPlayers(blackboard_struct *blackboard, int *num, rplayer_data *tempplayers){
	int i,j=0;

	//Tempplayers is assumed to be ALOTOF_PLAYERS long!!
	int length=ALOTOF_PLAYERS;

	if(num == NULL || blackboard == NULL || tempplayers == NULL)
		err_quit("NULL reference error!!!!");

	//Initialize invalide times in all the vector
	for(i=0;i<length;i++)
		tempplayers[i].time=-1;

	//Block semaphores
	sem_wait(&(blackboard->relative.r_sem));
	blackboard->relative.readers++;
	if(blackboard->relative.readers==1)
		sem_wait(&(blackboard->relative.mutex));
	sem_post(&(blackboard->relative.r_sem));

	//Copy data -- Using j as templayers index it returns a compact vector, with all used spaces at the beginning
	for(i=0;i<length;i++){
		if(blackboard->relative.players[i].time!=-1){ //copy only if the data is meaningful
			copyRPlayer(&(tempplayers[j]),&(blackboard->relative.players[i]));
			j++;
		}
	}
	*num=j;

	//Release semaphores
	sem_wait(&(blackboard->relative.r_sem));
	blackboard->relative.readers--;
	if(blackboard->relative.readers==0)
		sem_post(&(blackboard->relative.mutex));
	sem_post(&(blackboard->relative.r_sem));

	return;
}


void updateRelPlayer(blackboard_struct *blackboard, rplayer_data data){
	int found=0,i,j=-1, haveteam=1;
	rplayer_data *templayer;

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->relative.mutex));

	//check team field: if it points to an empty string or NULL set haveteam to 0
	if(strcmp("",data.team)==0 || data.team==NULL)
		haveteam--;

	//update data
	for(i=0;i<ALOTOF_PLAYERS && !found;i++){
		if((templayer=&(blackboard->relative.players[i]))->time!=-1){ //Check only if it is meaningful
			if(haveteam && strcmp(templayer->team,data.team)==0 && templayer->number==data.number)
				found++;
			if(!found && !haveteam && templayer->number==data.number)
				found++;

		} else if(j==-1)
			j=i; //j is the first free position in which it can put the player, if it doesn't match
	}

	if(found){ //substitute the old data with the new one
		copyRPlayer(templayer, &data);
	}else{ //insert it in the first free position
		copyRPlayer(&(blackboard->relative.players[j]),&data);
	}

	//Release sempahore
	sem_post(&(blackboard->relative.mutex));

	return;
}


void updateAbsPlayer(blackboard_struct *blackboard, aplayer_data data){
	int found=0,i,j=-1, haveteam=1;
	aplayer_data *templayer;

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->absolute.mutex));

	//check team field: if it points to an empty string or NULL set haveteam to 0
	if(strcmp("",data.team)==0 || data.team==NULL)
		haveteam--;

	//update data
	for(i=0;i<ALOTOF_PLAYERS && !found;i++){
		if((templayer=&(blackboard->absolute.players[i]))->time!=-1){ //Check only if it is meaningful
			//printf("Confronto %s %i con %s %i; haveteam: %i\n",templayer->team, templayer->number, data.team, data.number,haveteam);
			if(haveteam && strcmp(templayer->team,data.team)==0 && templayer->number==data.number)
				found++;
			if(!found && !haveteam && templayer->number==data.number)
				found++;

		} else if(j==-1)
			j=i; //j is the first free position in which it can put the player, if it doesn't match
	}

	if(found){ //substitute the old data with the new one
		copyAPlayer(templayer, &data);
	}else{ //insert it in the first free position
		copyAPlayer(&(blackboard->absolute.players[j]),&data);
	}

	//Release sempahore
	sem_post(&(blackboard->absolute.mutex));

	return;
}

void getAbsPlayers(blackboard_struct *blackboard, int *num, aplayer_data *tempplayers){
	int i,j=0;

	//tempplayers is assumed to be ALOTOF_PLAYERS long
	int length=ALOTOF_PLAYERS;

	if(blackboard == NULL || num == NULL || tempplayers == NULL)
		err_quit("NULL reference error!!!!");

	//Initialize invalide times in all the vector
	for(i=0;i<length;i++)
		tempplayers[i].time=-1;

	//Block semaphores
	sem_wait(&(blackboard->absolute.r_sem));
	blackboard->absolute.readers++;
	if(blackboard->absolute.readers==1)
		sem_wait(&(blackboard->absolute.mutex));
	sem_post(&(blackboard->absolute.r_sem));

	//Copy data -- Using j as templayers index it returns a compact vector, with all used spaces at the beginning
	for(i=0;i<length;i++){
		if(blackboard->absolute.players[i].time!=-1){ //copy only if the data is meaningful
			copyAPlayer(&(tempplayers[j]),&(blackboard->absolute.players[i]));
			j++;
		}
	}
	*num=j;

	//Release semaphores
	sem_wait(&(blackboard->absolute.r_sem));
	blackboard->absolute.readers--;
	if(blackboard->absolute.readers==0)
		sem_post(&(blackboard->absolute.mutex));
	sem_post(&(blackboard->absolute.r_sem));

	return;
}



rball_data getRelBall(blackboard_struct *blackboard){
	rball_data tempball={-1};

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semaphores
	sem_wait(&(blackboard->relative.r_sem));
	blackboard->relative.readers++;
	if(blackboard->relative.readers==1)
		sem_wait(&(blackboard->relative.mutex));
	sem_post(&(blackboard->relative.r_sem));

	//Copy data
	tempball=blackboard->relative.ball;

	//Release semaphores
	sem_wait(&(blackboard->relative.r_sem));
	blackboard->relative.readers--;
	if(blackboard->relative.readers==0)
		sem_post(&(blackboard->relative.mutex));
	sem_post(&(blackboard->relative.r_sem));

	return tempball;
}


void updateRelBall(blackboard_struct *blackboard, rball_data data){

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->relative.mutex));

	//update data
	blackboard->relative.ball=data;

	//Release sempahore
	sem_post(&(blackboard->relative.mutex));

	return;
}

aball_data getAbsBall(blackboard_struct *blackboard){
	aball_data tempball;

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semaphores
	sem_wait(&(blackboard->absolute.r_sem));
	blackboard->absolute.readers++;
	if(blackboard->absolute.readers==1)
		sem_wait(&(blackboard->absolute.mutex));
	sem_post(&(blackboard->absolute.r_sem));

	//Copy data
	tempball=blackboard->absolute.ball;

	//Release semaphores
	sem_wait(&(blackboard->absolute.r_sem));
	blackboard->absolute.readers--;
	if(blackboard->absolute.readers==0)
		sem_post(&(blackboard->absolute.mutex));
	sem_post(&(blackboard->absolute.r_sem));

	return tempball;
}


void updateAbsBall(blackboard_struct *blackboard, aball_data data){

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->absolute.mutex));

	//update data
	blackboard->absolute.ball=data;

	//Release sempahore
	sem_post(&(blackboard->absolute.mutex));

	return;
}


body_data getBody(blackboard_struct *blackboard){
	body_data tempdata;

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semaphores
	sem_wait(&(blackboard->body.r_sem));
	blackboard->body.readers++;
	if(blackboard->body.readers==1)
		sem_wait(&(blackboard->body.mutex));
	sem_post(&(blackboard->body.r_sem));

	//Copy data
	tempdata=blackboard->body.data;

	//Release semaphores
	sem_wait(&(blackboard->body.r_sem));
	blackboard->body.readers--;
	if(blackboard->body.readers==0)
		sem_post(&(blackboard->body.mutex));
	sem_post(&(blackboard->body.r_sem));

	return tempdata;
}


void updateBody(blackboard_struct *blackboard, body_data data){

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->body.mutex));

	//update data
	blackboard->body.data=data;

	//Release sempahore
	sem_post(&(blackboard->body.mutex));

	return;
}

ipotetic_offset getOffset(blackboard_struct *blackboard){
	ipotetic_offset tempoff;

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semaphores
	sem_wait(&(blackboard->body.r_sem));
	blackboard->body.readers++;
	if(blackboard->body.readers==1)
		sem_wait(&(blackboard->body.mutex));
	sem_post(&(blackboard->body.r_sem));

	//Copy data
	tempoff=blackboard->body.delta;

	//Release semaphores
	sem_wait(&(blackboard->body.r_sem));
	blackboard->body.readers--;
	if(blackboard->body.readers==0)
		sem_post(&(blackboard->body.mutex));
	sem_post(&(blackboard->body.r_sem));

	return tempoff;
}


void addOffset(blackboard_struct *blackboard, ipotetic_offset delta){
	double alfa;

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->body.mutex));

	alfa=blackboard->body.delta.d_alfa;
	alfa+= delta.d_alfa;

	//normalize alfa
	while(alfa>360)
		alfa-=360;
	while(alfa<-360)
		alfa+=360;
	if(alfa>180)
		alfa=-(360-alfa);
	if(alfa<-180)
		alfa=360+alfa;


	//update data
	blackboard->body.delta.d_x+=delta.d_x;
	blackboard->body.delta.d_y+=delta.d_y;
	blackboard->body.delta.d_alfa=alfa;


	//decrease precision, if possible!
	if(blackboard->body.delta.precision>=0.05)
		blackboard->body.delta.precision-=0.05;

	//DEBUG
	//err_msg("addOffset - OFFSET: x=%2.2f\ty=%2.2f\talfa=%2.2f\tprecision=%1.2f",blackboard->body.delta.d_x,blackboard->body.delta.d_y,blackboard->body.delta.d_alfa, blackboard->body.delta.precision);


	//Release sempahore
	sem_post(&(blackboard->body.mutex));

	return;
}

void applyOffset(blackboard_struct *blackboard){
	double alfa;

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->body.mutex));

	//DEBUG
	//err_msg("applyOffset - BODY: x=%2.2f\ty=%2.2f\talfa=%2.2f",blackboard->body.data.x,blackboard->body.data.y,blackboard->body.data.body_angle);
	//err_msg("applyOffset - OFFSET: x=%2.2f\ty=%2.2f\talfa=%2.2f\tprecision=%1.2f",blackboard->body.delta.d_x,blackboard->body.delta.d_y,blackboard->body.delta.d_alfa, blackboard->body.delta.precision);

	alfa=blackboard->body.data.body_angle;
	alfa+= blackboard->body.delta.d_alfa;

	//normalize alfa
	while(alfa>360)
		alfa-=360;
	while(alfa<-360)
		alfa+=360;
	if(alfa>180)
		alfa=-(360-alfa);
	if(alfa<-180)
		alfa=360+alfa;

	//update body position
	blackboard->body.data.x+=blackboard->body.delta.d_x;
	blackboard->body.data.y+=blackboard->body.delta.d_y;

	blackboard->body.data.body_angle=alfa;

	//set offset data to 0
	blackboard->body.delta.d_x=0;
	blackboard->body.delta.d_y=0;
	blackboard->body.delta.d_alfa=0;
	/*if(blackboard->body.delta.precision<0.1){
		err_msg("WARNING: precision fault!\n");
	}*/

	//TO BE NOTED that precision is not touched, until reset precision go on decreasing!

	//DEBUG
	//err_msg("applyOffset - NEW BODY: x=%2.2f\ty=%2.2f\talfa=%2.2f",blackboard->body.data.x,blackboard->body.data.y,blackboard->body.data.body_angle);
	//err_msg("applyOffset - NEW OFFSET: x=%2.2f\ty=%2.2f\talfa=%2.2f\tprecision=%1.2f",blackboard->body.delta.d_x,blackboard->body.delta.d_y,blackboard->body.delta.d_alfa, blackboard->body.delta.precision);




	//Release sempahore
	sem_post(&(blackboard->body.mutex));

	return;
}


void resetOffset(blackboard_struct *blackboard){

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->body.mutex));

	//update data
	blackboard->body.delta.d_x=0;
	blackboard->body.delta.d_y=0;
	blackboard->body.delta.d_alfa=0;

	blackboard->body.delta.precision=1; //Restore max precision

	//Release sempahore
	sem_post(&(blackboard->body.mutex));

	return;
}

ai_data getAi(blackboard_struct *blackboard){
	ai_data tempdata;

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semaphores
	sem_wait(&(blackboard->ai.r_sem));
	blackboard->ai.readers++;
	if(blackboard->ai.readers==1)
		sem_wait(&(blackboard->ai.mutex));
	sem_post(&(blackboard->ai.r_sem));

	//Copy data
	tempdata=blackboard->ai.data;

	//Release semaphores
	sem_wait(&(blackboard->ai.r_sem));
	blackboard->ai.readers--;
	if(blackboard->ai.readers==0)
		sem_post(&(blackboard->ai.mutex));
	sem_post(&(blackboard->ai.r_sem));

	return tempdata;
}


void updateAi(blackboard_struct *blackboard, ai_data data){

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->ai.mutex));

	//update data
	blackboard->ai.data=data;
	blackboard->ai.valid=1;

	//Release semaphore
	sem_post(&(blackboard->ai.mutex));

	return;
}

void nopAi(blackboard_struct *blackboard){
	
	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->ai.mutex));

	//update data
	blackboard->ai.valid=-1;

	//Release semaphore
	sem_post(&(blackboard->ai.mutex));

	return;
}


void updateTeam(blackboard_struct *blackboard, char *team){

	if(blackboard ==  NULL || team == NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->mutex));

	//Copy data
	strncpy(blackboard->team,team,MAX_TEAMNAME_LENGHT);
	blackboard->team[MAX_TEAMNAME_LENGHT]='\0';

	//Release sempahore
	sem_post(&(blackboard->mutex));

	return;
}


char *GetTeam(blackboard_struct *blackboard, char *team){

	if(blackboard ==  NULL || team == NULL)
		err_quit("NULL reference error!!!!");

	//Block semaphores
	sem_wait(&(blackboard->r_sem));
	blackboard->readers++;
	if(blackboard->readers==1)
		sem_wait(&(blackboard->mutex));
	sem_post(&(blackboard->r_sem));

	//Copy data
	strncpy(team,blackboard->team, sizeof(team));


	//Release semaphores
	sem_wait(&(blackboard->r_sem));
	blackboard->readers--;
	if(blackboard->readers==0)
		sem_post(&(blackboard->mutex));
	sem_post(&(blackboard->r_sem));

	return team;
}

void updateNumber(blackboard_struct *blackboard, int num){

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->mutex));

	blackboard->number=num;
	//Release sempahore
	sem_post(&(blackboard->mutex));

	return;
}


int GetNumber(blackboard_struct *blackboard){
	int number;

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semaphores
	sem_wait(&(blackboard->r_sem));
	blackboard->readers++;
	if(blackboard->readers==1)
		sem_wait(&(blackboard->mutex));
	sem_post(&(blackboard->r_sem));

	//Copy data
	number=blackboard->number;

	//Release semaphores
	sem_wait(&(blackboard->r_sem));
	blackboard->readers--;
	if(blackboard->readers==0)
		sem_post(&(blackboard->mutex));
	sem_post(&(blackboard->r_sem));

	return number;
}


void updateSide(blackboard_struct *blackboard, char side){

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->mutex));

	blackboard->side=side;

	//Release sempahore
	sem_post(&(blackboard->mutex));

	return;
}

char getSide(blackboard_struct *blackboard){
	char side;

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semaphores
	sem_wait(&(blackboard->r_sem));
	blackboard->readers++;
	if(blackboard->readers==1)
		sem_wait(&(blackboard->mutex));
	sem_post(&(blackboard->r_sem));

	side=blackboard->side;

	//Release semaphores
	sem_wait(&(blackboard->r_sem));
	blackboard->readers--;
	if(blackboard->readers==0)
		sem_post(&(blackboard->mutex));
	sem_post(&(blackboard->r_sem));


	return side;
}



void updatePLaymode(blackboard_struct *blackboard, int playmode){

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->mutex));

	blackboard->playmode=playmode;

	//DEBUG
	//err_msg("updatePlaymode - New Playmode: %i",playmode);

	//Release sempahore
	sem_post(&(blackboard->mutex));

	return;
}

int getPLaymode(blackboard_struct *blackboard){
	int mode;

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semaphores
	sem_wait(&(blackboard->r_sem));
	blackboard->readers++;
	if(blackboard->readers==1)
		sem_wait(&(blackboard->mutex));
	sem_post(&(blackboard->r_sem));

	mode=blackboard->playmode;

	//Release semaphores
	sem_wait(&(blackboard->r_sem));
	blackboard->readers--;
	if(blackboard->readers==0)
		sem_post(&(blackboard->mutex));
	sem_post(&(blackboard->r_sem));


	return mode;
}

int setTime(blackboard_struct *blackboard, int time){

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semahpore
	sem_wait(&(blackboard->mutex));

	//Copy data
	blackboard->now=time;

	//Release sempahore
	sem_post(&(blackboard->mutex));


	return 0;
}

int getTime(blackboard_struct *blackboard){
	int time;

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semaphores
	sem_wait(&(blackboard->r_sem));
	blackboard->readers++;
	if(blackboard->readers==1)
		sem_wait(&(blackboard->mutex));
	sem_post(&(blackboard->r_sem));

	//Copy data
	time=blackboard->now;

	//Release semaphores
	sem_wait(&(blackboard->r_sem));
	blackboard->readers--;
	if(blackboard->readers==0)
		sem_post(&(blackboard->mutex));
	sem_post(&(blackboard->r_sem));

	return time;
}

int aiStatus(blackboard_struct *blackboard){
	int status;

	if(blackboard ==  NULL)
		err_quit("NULL reference error!!!!");

	//Block semaphores
	sem_wait(&(blackboard->ai.r_sem));
	blackboard->ai.readers++;
	if(blackboard->ai.readers==1)
		sem_wait(&(blackboard->ai.mutex));
	sem_post(&(blackboard->ai.r_sem));

	//Copy data
	status=blackboard->ai.valid;

	//Release semaphores
	sem_wait(&(blackboard->ai.r_sem));
	blackboard->ai.readers--;
	if(blackboard->ai.readers==0)
		sem_post(&(blackboard->ai.mutex));
	sem_post(&(blackboard->ai.r_sem));

	return status;
}

int printRelFlags(blackboard_struct *blackboard,char *buffer){
	int i;
	rflag_data flag;
	char tempstr[10000+1];
	if(blackboard == NULL || buffer == NULL)
		err_quit("NULL reference error!!!!");

	buffer[0]='\0';
	for(i=FLAG_T_L_50;i<FLAG_B_R_50;i++){
		flag=getRelFlag(i, blackboard);
		if(flag.time!=-1){
			// WARNING!!! The use of this function is not safe. Buffer overflow danger!
			sprintf(tempstr, "###Relative Flags: %i\n\ttime=%i\tdist=%f\talfa=%f\n",i, flag.time, flag.distance, flag.direction);
			strcat(buffer, tempstr);
		}
	}
	return i;

}

int printAbsFlags(blackboard_struct *blackboard,char *buffer){
	int i;
	aflag_data flag;
	char tempstr[10000+1];
	if(blackboard == NULL || buffer == NULL)
		err_quit("NULL reference error!!!!");

	buffer[0]='\0';
	for(i=FLAG_T_L_50;i<FLAG_B_R_50;i++){
		flag=getAbsFlag(i, blackboard);
		if(flag.time!=-1){
			// WARNING!!! The use of this function is not safe. Buffer overflow danger!
			sprintf(tempstr, "###Absolute Flags: %i\n\t%i\t%f\t%f\n",i, flag.time, flag.x, flag.y);
			strcat(buffer, tempstr);
		}
	}
	return i;

}

