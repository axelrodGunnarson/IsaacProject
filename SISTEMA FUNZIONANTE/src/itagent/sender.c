/*
 *  sender.c
 *  Isaac_Agent
 *
 *  Created by erix on 03/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "errlib.h"
#include "blackboard.h"
#include "libserver.h"

#include "sender.h"


//Wrapper di sendMsg: before sending action lisp command to server update offset!
// ATTENTION: It doesn't control the right syntax. Command MUST be corrected when sent, or misunderstanding will be present 
int sendAction(server_data *server, char *buffer){
	float p1=0, p2=0, dist;
	double alfa=0;
	int args;
	//int time=0;
	body_data body;
	ipotetic_offset delta={0};
	
	double err;

	//DEBUG
	//time=getTime(&blackboard);
	//err_msg("%i - sendAction() - Sending: %s",time,buffer);
	
	if(strncmp(buffer, "(move", 5)==0){
		args=sscanf(buffer,"(move %f %f)",&p1, &p2);
		if(args!=2){
			err_msg("Move command syntax wrong!!!");
			return -1;
		}
			
		body=getBody(&blackboard);
		body.x=p1;
		body.y=p2;
		updateBody(&blackboard, body);
		
		//DEBUG
		//body=getBody(&blackboard);
		//err_msg("sendAction - BODY: x=%2.2f\ty=%2.2f\talfa=%2.2f",body.x, body.y,body.body_angle);
	}
	
	if(strncmp(buffer, "(dash", 5)==0){
		args=sscanf(buffer,"(dash %f)",&p1);
		if(args!=1){
			err_msg("Dash command syntax wrong!!!");
			return -1;
		}
		
		body=getBody(&blackboard);
		alfa=body.body_angle;
		
		//very drafted!!!
		
		alfa=alfa*M_PI/180;
		dist=p1/100;

		delta.d_x=(float)cos(alfa)*dist; 
		delta.d_y=(float)sin(alfa)*dist;
		delta.d_alfa=0;
		
		addOffset(&blackboard, delta);
		
		//DEBUG
		//err_msg("sendAction - NEW OFFSET: x=%2.2f\ty=%2.2f\talfa=%2.2f", delta.d_x,delta.d_y, delta.d_alfa);
		//delta=getOffset(&blackboard);
		//err_msg("sendAction - TOTAL OFFSET: x=%2.2f\ty=%2.2f\talfa=%2.2f\tprecision:%1.2f", delta.d_x,delta.d_y, delta.d_alfa,delta.precision);
		
		
	}

	if(strncmp(buffer, "(turn", 5)==0){
		args=sscanf(buffer,"(turn %f)",&p1);
		if(args!=1){
			err_msg("Turn command syntax wrong!!!");
			return -1;
		}
		

		//adding error for test purpose
		
		err=(float)(rand()%10);
		if((rand()%2)==1)
			err=-err;

		delta.d_x=0;
		delta.d_y=0;
		delta.d_alfa=p1+err;
		
		addOffset(&blackboard, delta);
		
		//DEBUG
		//err_msg("sendAction - NEW OFFSET: x=%2.2f\ty=%2.2f\talfa=%2.2f", delta.d_x,delta.d_y, delta.d_alfa);
		//delta=getOffset(&blackboard);
		//err_msg("sendAction - TOTAL OFFSET: x=%2.2f\ty=%2.2f\talfa=%2.2f\tprecision:%1.2f", delta.d_x,delta.d_y, delta.d_alfa,delta.precision);
		
	}
	
	
	//Eventually send the action
	sendMsg(server, buffer);
	return 0;	
		
		
	
}
