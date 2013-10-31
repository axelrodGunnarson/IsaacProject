/*
 *  rules.c
 *  Isaac_Agent
 *
 *  Created by erix on 04/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "../include/ai.h"
#include "../include/errlib.h"
#include "../include/localization.h"


#define GOT_BALL_THRES 10
#define NEAR_OPPONENT_GOAL_THRESHOLD 30

extern sem_t s_ai;
extern sem_t s_localizationComplete;
extern sem_t s_aiComplete;

int aiThread(){

    int situation[N_CONDITIONS]={0};
    //int i,time;
    //int oldsit;

    double nsec;
    clock_t start,end;


	while(1){
		
		sem_wait(&s_ai);
		
		start=clock();
		sem_wait(&s_localizationComplete);
		//err_msg("AI\n");

		//do stuff..
        //time=getTime(&blackboard);
		//oldsit=situation[0];
		valuateSituation(situation);
		
		//Choose the action to be done
		//
		int mode=getPLaymode(&blackboard);
		if(mode==PLAY_ON){	
			//int time=getTime(&blackboard);
			aball_data ball=getAbsBall(&blackboard);
			if(ball.time!=-1){
				ai_data ai;
				ai.dest_x=ball.x;
				ai.dest_y=ball.y;
				ai.dest_alfa=0;
				updateAi(&blackboard,ai);
				//printf("%i - new destination: x=%f y=%f 0\n",time,ai.dest_x, ai.dest_y);
			}else{
				//printf("%i - Ball not seen!\n",time);
				nopAi(&blackboard);
			
			}
		}


		//
		//
		
		//DEBUG
/*		if(oldsit != situation[GOT_BALL] && situation[GOT_BALL]==1)
            printf("GOT BALL!!\n");
		if(oldsit != situation[GOT_BALL] && situation[GOT_BALL]==0)
            printf("LOST BALL!!\n");*/

		/*printf("aiThread() - Printing situation at %i: ",time);
        for(i=0;i<N_CONDITIONS;i++)
            printf("%i",situation[i]);
        printf("\n");*/
		sem_post(&s_aiComplete);
		end=clock();

		nsec=(((double)end-start)/CLOCKS_PER_SEC)*(1e9);
        if(nsec>SIMULATION_STEP_MS*(1e6))
			err_msg("aiThread() - DANGER, time is too fast!");

	}


}


int valuateSituation(int *situation){

	situation[GOT_BALL]=checkGotBall();
	situation[NEAR_OPPONENT_GOAL]=checkNearOpponentGoal();

	return -1;
}

int checkGotBall(){
	double p_x,p_y,b_x,b_y;
	body_data body;
	aball_data ball;
    double delta;

	//valuate GOT_BALL
	body=getBody(&blackboard);
	p_x=body.x;
	p_y=body.y;
	ball=getAbsBall(&blackboard);
	b_x=ball.x;
	b_y=ball.y;

	delta=distance(p_x,b_x,p_y,b_y);

	/*d_x=abs(p_x-b_x);
	d_y=abs(p_y-b_y);
    delta=sqrt(pow(d_x,2)+pow(d_y,2));*/
    //printf("checkGotBall() - Debug info: p_x=%f p_y=%f b_x=%f b_y=%f delta=%f soglia=%i\n",p_x,p_y,b_x,b_y,delta,GOT_BALL_THRES);
    if(delta<GOT_BALL_THRES)
        return 1;
    return 0;

}


int checkNearOpponentGoal(){
	body_data body=getBody(&blackboard);
	char side=getSide(&blackboard);
	double delta;

	//printf("Body: ")

	if(side=='l')
		delta=distance(body.x,body.y,vett_offset[FLAG_G_R].offset_x-(FIELD_LENGTH/2),vett_offset[FLAG_G_R].offset_y-(FIELD_HEIGHT/2));
	if(side=='r')
		delta=distance(body.x,body.y,vett_offset[FLAG_G_L].offset_x-(FIELD_LENGTH/2),vett_offset[FLAG_G_L].offset_y-(FIELD_HEIGHT/2));
	
	if(delta < NEAR_OPPONENT_GOAL_THRESHOLD){
		//printf("Sono vicino alla porta avversaria!\n");
		return 1;
	}



	return 0;

}

double distance(double x1, double y1, double x2, double y2){
	double d_x,d_y;

	d_x=abs(x1-x2);
	d_y=abs(y1-y2);
    return sqrt(pow(d_x,2)+pow(d_y,2));

}

