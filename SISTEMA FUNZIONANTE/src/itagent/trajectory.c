/*
 *  trajectory.c
 *  Isaac_Agent
 *
 *  Created by anthos89.
 *  Copyright 2011 __YourCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../include/trajectory.h"
#include "../include/errlib.h"
#include "../include/sender.h"
#include "../include/rules.h"

#define REACH_RADIUS 0.1

double a = 0;
char last_act ='d';
extern sem_t s_trajectory;
extern sem_t s_aiComplete;
extern sem_t s_trajectoryComplete;

/* TO DO
	- Correzione codice e riordino
 */


//this is a test function, with a wrapper to start testing trajectory main function
void trajectory_thread(server_data *server){

	clock_t start,end;
	double nsec;

	//int now;


	while(1){
	    sem_wait(&s_trajectory);
		
		start=clock();
		sem_wait(&s_aiComplete); //wait for localization to be completed before elaborate!
		//err_msg("trajectory!\n");		

		//now=getTime(&blackboard);

		//DEBUG
		//err_msg("It's Time %i and check next trajectories!!",now);


		trajectory(server);
		sem_post(&s_trajectoryComplete);

		end=clock();

		nsec=(((double)end-start)/CLOCKS_PER_SEC)*(1e9);
        if(nsec>SIMULATION_STEP_MS*(1e6))
			err_msg("trajectory() - DANGER, time is too fast!");

	}
}

void trajectory(server_data *server){

	int i;
	int players;

	//if final position is busy from another player
	boolean BusyP = FALSE;

	//for to know when the angle should be changed
	boolean Go;
	boolean modify_ang=FALSE;

	boolean ReachedP=FALSE; //reached position
	float d; //distance between two points
	double ang_tmp, x_tmp, y_tmp;
	aplayer_data *data; //possible obstacles

	//for check equation obstacles
	allObstacles ob;

	//number of players in the field
	int nPlayers=0;

	//in new obstacles that are the obstacles near on the trajectory
	//including the obstacle that stands in the path
	allObstacles newOb;

	//the initial and final position
	Point2D start, end;

	Point2D allPoints[3];
	//bezier points
	Point2D b[4];

	Point2D curve[CurvePoints];
	aplayer_data cut;//dummy variable
	Point2D ctrlCurve;

	int f;//flag for fromTo function

	double x,y,ang,x2,y2,ang2;
	ai_data ai;
	body_data body;

	//If AI decisions are inconsistent stop here
	if(aiStatus(&blackboard)==-1){
		int mode=getPLaymode(&blackboard);
		int time=getTime(&blackboard);
		if(mode == PLAY_ON && (time%3)==0 )
			sendAction(server,"(turn 10)");
		return;
	}

	//Getting updated position
	body=getBody(&blackboard);
	x=body.x;
	y=body.y;
	ang=body.body_angle;


	//Getting updated destination
	ai=getAi(&blackboard);
	x2=(double)ai.dest_x;
	y2=(double)ai.dest_y;
	ang2=(double)ai.dest_alfa;

	if(inCircle(x,y,x2,y2,REACH_RADIUS)){
//		if(ang==ang2)
		if(tolerance_angle(ang,ang2)){
			int time=getTime(&blackboard);
			printf("%i - posizione raggiunta\n",time);
			return;
		}
		else
			modify_ang=TRUE;
	}


	//err_msg("Nuovo comando strategico ricevuto\n");
	
	//int time=getTime(&blackboard);
	//printf("last act vale %c\n",last_act);
	//printf("%i - Trajectory() - body.x=%f\tbody.y=%f\tbody_angle=%f\n",time,x,y,ang);
//	err_msg("x2=%f\ty2=%f\tangle2=%f\n",x2,y2,ang2);

	data=(aplayer_data*)malloc(ALOTOF_PLAYERS*(sizeof(aplayer_data)));

	getAbsPlayers(&blackboard,&players,data);
	/**printf("id=%d  x=%f  y=%f\n",0,data[0].x,data[0].y);
	printf("id=%d  x=%f  y=%f\n",1,data[1].x,data[1].y);
	printf("id=%d  x=%f  y=%f\n",2,data[2].x,data[2].y);**/

	Go = FALSE;

	while(!ReachedP && !BusyP)
	{
		ob=ctrlObstacles(x, y, x2, y2, &nPlayers);

		d=ctrlDist(x, y, x2, y2);

		if(ob.n_eq == 0)
		{
			if(d > THRESHOLD)
			{
				ang_tmp = atan((y2-y)/(x2-x));
				ang_tmp = ang_tmp*180/M_PI;

				x_tmp = findPoint(x, y, x2, y2, (double)STEP, 'x');
				y_tmp = findPoint(x, y, x2, y2, (double)STEP, 'y');

				//printf("%f\t%f\n", x_tmp, y_tmp);
				if(x2<x && y2>y)
					ang_tmp = 180 + ang_tmp;
				else if(x2<x && y2<y)
					ang_tmp = ang_tmp - 180;
				f=fromTo(server, x, y, ang, x_tmp, y_tmp, ang_tmp, modify_ang);

				if(f) {
					free(data);
					return;
				}

				x = x_tmp;
				y = y_tmp;
				ang = ang_tmp;
			}
			else
			{
				f=fromTo(server, x, y, ang, x2, y2, ang2, modify_ang);

				if(f){
					free(data);
					return;
				}

				ReachedP = TRUE;
			}
		}
		else
		{
			newOb = initNewOb(ob.eq_id[0]);

			start.x = x;
			start.y = y;

			end.x = x2;
			end.y = y2;

			insertNewOb(ob,&newOb);

			for(i=0; i<newOb.n_eq; i++)
				checkNewOb(newOb.eq_id[i], cut, 0, data, nPlayers, &newOb, start);

			findAllPoints(start, end, newOb, allPoints);

			/***printf("id=%d  x=%f  y=%f\n",0,allPoints[0].x,allPoints[0].y);
			printf("id=%d  x=%f  y=%f\n",1,allPoints[1].x,allPoints[1].y);
			printf("id=%d  x=%f  y=%f\n",2,allPoints[2].x,allPoints[2].y);***/

			findBezierPoints(allPoints, start, end, newOb, b);

			/***printf("id=%d  x=%f  y=%f\n",0,b[0].x,b[0].y);
			printf("id=%d  x=%f  y=%f\n",1,b[1].x,b[1].y);
			printf("id=%d  x=%f  y=%f\n",2,b[2].x,b[2].y);
			printf("id=%d  x=%f  y=%f\n",3,b[3].x,b[3].y);***/

			/* if distance between b[0] and start > [THRESHOLD/THRESHOLD+STEP/STEP]
			 * go simply ahead */
			if(ctrlDist(x, y, b[0].x, b[0].y) > STEP)
			{
				ang_tmp = atan((y2-y)/(x2-x));
				ang_tmp = ang_tmp*180/M_PI;

				if(x2 < x && y2 > y)
				{
					ang_tmp *= -1;
					ang_tmp += 90;
				}

				x_tmp = findPoint(x, y, x2, y2, (double)STEP, 'x');
				y_tmp = findPoint(x, y, x2, y2, (double)STEP, 'y');

				if(Go)
					Go = FALSE;

				f=fromTo(server, x, y, ang, x_tmp, y_tmp, ang_tmp, modify_ang);

				if(f) {
					free(data);
					return;
				}

				x = x_tmp;
				y = y_tmp;
				ang = ang_tmp;
			}
			else
			{
				if(b[0].x!=x || b[0].y!=y)
					b[0] = start;

				ComputeBezier(b, CurvePoints, curve);

				/* now our robot should to go on curve[0] and after
				 * restart the while but since the obstacles are
				 * stopped the robot continue until
				 * curve[CurvePoints-1] is reached */

				ctrlCurve = becomeCurved(server, curve, data, nPlayers,
								newOb, &ang, modify_ang);

				if(ctrlCurve.x == -12345 && ctrlCurve.y == -12345){
					free(data);
					return;
				}

				//system("sleep 1");

				//ang = a;

				/* if ctrlCurve == FALSE it means that the final
				 * point is not reached
				 * and if in the same time y2 and x2 is into an obstacle
				 * we should exit */

				if(ctrlCurve.x != curve[CurvePoints-1].x &&
					ctrlCurve.y != curve[CurvePoints-1].y &&
					 curve[CurvePoints-1].x == x2 && curve[CurvePoints-1].y == y2)
				{
					BusyP = TRUE;
					printf("Position Not Reached!!\n");
					printf("In the final position there is already another player!!\n");
				}
				else if(ctrlCurve.x == curve[CurvePoints-1].x &&
						 ctrlCurve.y == curve[CurvePoints-1].y &&
						  curve[CurvePoints-1].x == x2 && curve[CurvePoints-1].y == y2)
				{
					ReachedP = TRUE;
					printf("Position Reached with success\n");
				}
				else
				{
					x = ctrlCurve.x;
					y = ctrlCurve.y;

					Go = TRUE;
				}
			}
		}
	}
	free(data);
}

double setTurn(double a, double ang)
{
	double a_tmp=0;

	if(ang == 0)
		return -a;
	else if(a < 0 && ang > 0 && ang > (a + 180))
		a_tmp = -180 -a -180 +ang;
	else if(a > 0 && ang < 0 && ang < (a -180))
		a_tmp = 180 -a + 180 +ang;
	else
		a_tmp = -a + ang;

	return a_tmp;
}

void modify_kick(double *power, double *direction)
{
	if (*direction < -180)
                *direction += 360.0;
	if (*direction > 180)
			*direction -= 360.0;
	if (*power < 0)
			*power = 0.0;
	if (*power > 100)
			*power = 100;
}

// Modificata tanto da Luk!!!!

int fromTo(server_data *server, double x, double y, double ang, double x2, double y2, double ang2, boolean modify_ang)
{
	int i; //i=index
	double d_t; //d_t = direction temp
	double direction;
	double d; // d=distance
	double v; //v=integer val of d
	char buffer[MAX_MSG_SIZE+1]={ 0 };

	//LUK
	double prova_ang=0;
	//struct timespec pause;

	//pause.tv_sec=0;
	//pause.tv_nsec=100000000;

	v=(x2-x)*(x2-x)+(y2-y)*(y2-y);

	d = sqrt(v);

	v = (int) d;

	a=ang;

	if((x2-x)==0)
	{
		if(y2>y && !tolerance_angle(a,(double)90))
		{
			/*a=90*/

			if(a >= -180 && a < -90)
				prova_ang=(double)(-1*(180+a+90));
			else
				prova_ang=(double)(90-a);

			if(prova_ang > THRESHOLD_TURN) prova_ang=THRESHOLD_TURN;
			if(prova_ang < -THRESHOLD_TURN) prova_ang=-THRESHOLD_TURN;

			snprintf(buffer,MAX_MSG_SIZE, "(turn %f)",prova_ang);
			sendAction(server, buffer);
			last_act='t';
			
			//nanosleep(&pause,NULL); // sleep for 0.1 sec
			return 1;
		}
		else if(y2<y && !tolerance_angle(a,(double)-90))
		{
			/*a=-90*/
			if(a >= 90 && a <= 180)
				prova_ang=(double)(180-a+90);
			else
				prova_ang=(double)(-90-a);

			if(prova_ang > THRESHOLD_TURN) prova_ang=THRESHOLD_TURN;
			if(prova_ang < -THRESHOLD_TURN) prova_ang=-THRESHOLD_TURN;

			snprintf(buffer,MAX_MSG_SIZE, "(turn %f)",prova_ang);
			sendAction(server, buffer);
			last_act='t';
			//nanosleep(&pause,NULL); // sleep for 0.1 sec
			return 1;
		}

		if(v!=0)
		{
			snprintf(buffer,MAX_MSG_SIZE, "(dash 100)");
			sendAction(server, buffer);
			last_act='d';
			//nanosleep(&pause,NULL); // sleep for 0.1 sec
			/****getchar();****/

			return 1;
		}
		else if(d!=0){ //sendDash(server, (double) ((d-v)*100));
			snprintf(buffer,MAX_MSG_SIZE, "(dash %f)",(double)((d)*100));
			sendAction(server, buffer);
			last_act='d';
			//nanosleep(&pause,NULL); // sleep for 0.1 sec

			return 1;
		}
	}
	else
	{
		direction=atan((y2-y)/(x2-x));
		direction=direction*180/M_PI;

		if(x2<x && y2<y)
			direction = - 180 + direction;

		if(x2<x && y2>y)
			direction = 180 + direction;

		//direction=setTurn(&a,direction);
		/***printf("direction=%f\n",direction);***/
		//d_t=direction;

		if(modify_ang){
			d_t=setTurn(ang,ang2);

			if(d_t > THRESHOLD_TURN) d_t=THRESHOLD_TURN;
			if(d_t < -THRESHOLD_TURN) d_t=-THRESHOLD_TURN;

			snprintf(buffer,MAX_MSG_SIZE, "(turn %f)", d_t);
			sendAction(server, buffer);
			last_act='t';

			return 1;
		}

//		if(DIGIT_DECIMAL(direction)!=DIGIT_DECIMAL(ang))//direction ? d_t
		if(!tolerance_angle(direction,ang))
		{

			d_t=setTurn(a,direction);

			if(d_t > THRESHOLD_TURN) d_t=THRESHOLD_TURN;
			if(d_t < -THRESHOLD_TURN) d_t=-THRESHOLD_TURN;

			//sendTurn(server, d_t);
			snprintf(buffer,MAX_MSG_SIZE, "(turn %f)",d_t);
			sendAction(server, buffer);
			last_act='t';

			//nanosleep(&pause,NULL); // sleep for 0.1 sec

			return 1;
		}

		for(i=0; i<v; i++)
		{
			//sendDash(server, (double)100);
			snprintf(buffer,MAX_MSG_SIZE, "(dash 100)");
			sendAction(server, buffer);
			last_act='d';

			//nanosleep(&pause,NULL); // sleep for 0.1 sec
			/****getchar();****/
			return 1;
		}
		if(d!=v){ //sendDash(server, (double) ((d-v)*100));
			snprintf(buffer,MAX_MSG_SIZE, "(dash %f)",(double)((d-v)*100));
			sendAction(server, buffer);
			last_act='d';

			return 1;
		}
	}

	ang2=setTurn(a,ang2);

	if(!tolerance_angle(ang,ang2)){
	if(ang2 > THRESHOLD_TURN) ang2=THRESHOLD_TURN;
	if(ang2 < -THRESHOLD_TURN) ang2=-THRESHOLD_TURN;

	snprintf(buffer,MAX_MSG_SIZE, "(turn %f)", ang2);
	sendAction(server, buffer);
	last_act='t';

	return 1;
	}

	return 0;
}

allObstacles ctrlObstacles(double x0, double y0, double x1, double y1, int *nPlayers)
{
	aplayer_data *data;
	// count says how many obstacles there are
	int i, count=0;
	allObstacles ob;
	float R; //radius
	float delta;
	float xc,yc;
	float A, B, C, t;
	float xTmp, yTmp; // x and y temporary
	float distFromMe;
	Point2D p;
	float tx0, tx1, ty0, ty1;

	int players;

	boolean ctrlExit = FALSE;

	data=(aplayer_data*)malloc(ALOTOF_PLAYERS*(sizeof(aplayer_data)));

	getAbsPlayers(&blackboard,&players,data);
/**	printf("id=%d  x=%f  y=%f\n",0,data[0].x,data[0].y);
	printf("id=%d  x=%f  y=%f\n",1,data[1].x,data[1].y);
	printf("id=%d  x=%f  y=%f\n",2,data[2].x,data[2].y);**/

	for(i=0; i<ALOTOF_PLAYERS && !ctrlExit ; i++)
	{
		/*
		if (i < 10)
		{
			printf("data[%d].time = %d\n",i,data[i].time);
			printf("data[%d].x = %f\n",i,data[i].x);
			printf("data[%d].y = %f\n\n",i,data[i].y);
		}
		*/
		if(data[i].time != -1)/********/
		{
			(*nPlayers)++;

			xc = data[i].x;
			yc = data[i].y;

			// first finds radius of the circle
			R = findRadius((float)x0, (float)y0, xc, yc);
			/***printf("R = %f\n", R);***/

			// check the intersection

			A = pow((x1-x0),2) + pow((y1-y0),2);
			B = (2*x0-2*xc)*(x1-x0) + (y1-y0)*(2*y0-2*yc);
			C = x0*x0 + xc*xc - 2*x0*xc + y0*y0 + yc*yc -2*y0*yc - R*R;

			delta = B*B -4*A*C;
			/***printf("delta = %f\n", delta);***/

			// if delta < 0 there are none intersection
			// if delta = 0 the safety radius allows us to move without collision
			// if delta > 0 this circle is an obstacle on the line

			if(delta > 0)
			{
				/***printf("R = %f\n", R);**/

				t = -1*(B + sqrt(delta))/(2*A);

				xTmp = x0 + ((x1-x0)*t);
				yTmp = y0 + ((y1-y0)*t);

				p.x = xTmp;
				p.y = yTmp;

				distFromMe = ctrlDist(x0, y0, xTmp, yTmp);

				/***printf("xTmp1 = %f\nyTmp1 = %f\n", xTmp, yTmp);**/

				if(x1 < x0)
				{
					tx0 = x1;
					tx1 = x0;
				}
				else
				{
					tx0 = x0;
					tx1 = x1;
				}

				if(y1 < y0)
				{
					ty0 = y1;
					ty1 = y0;
				}
				else
				{
					ty0 = y0;
					ty1 = y1;
				}

				if(xTmp <= tx1 && xTmp >= tx0 && yTmp <= ty1 && yTmp >= ty0)
				{
					t = -1*(B - sqrt(delta))/(2*A);

					xTmp = x0 + ((x1-x0)*t);
					yTmp = y0 + ((y1-y0)*t);

					/***printf("xTmp2 = %f\nyTmp2 = %f\n\n", xTmp, yTmp);***/

					/*Si controlla solo se la prima intersezione è interna la traiettoria
					 * e se così fosse la seconda bisogna vedere solo se è successiva
					 * al punto iniziale perchè l'ostacolo potrebbe trovarsi vicino al
					 * punto finale della traiettoria*/
					/*It only checks whether the first intersection is inside the trajectory
					 * And if so the latter should only see if a later
					 * To the starting point because it could be close to the obstacle
					 * End point of the trajectory*/
					if(xTmp >= tx0 && yTmp >= ty0)
					{
						ob.eq_id[count].xc = xc;
						ob.eq_id[count].yc = yc;
						ob.eq_id[count].r = R;
						ob.eq_id[count].fromMe = distFromMe;
						/* fromMe is the first intersection and it is
						 * important to know the order in which there are the obstacles
						 * in a way that makes it easier to order*/
						ob.eq_id[count].x1 = p.x;
						ob.eq_id[count].y1 = p.y;
						ob.eq_id[count].x2 = xTmp;
						ob.eq_id[count].y2 = yTmp;

						count++;
					}
				}
			}
		}
		else
			ctrlExit = TRUE;
	}

	ob.n_eq = count;
	/***
	printf("number obstacles:\t%d\n", ob.n_eq);
	***/
	/***printf("before sort\n");
	for(count=0; count<ob.n_eq; count++)
		printf("xc[%d] = %f\tyc[%d] = %f\n",count, ob.eq_id[count].xc, count, ob.eq_id[count].yc);
	printf("\n\n");
	***/

	sort(ob.eq_id, 0, ob.n_eq-1);

	free(data);

	/***
	printf("after sort\n");
	for(count=0; count<ob.n_eq; count++)
		printf("xc[%d] = %f\tyc[%d] = %f\n",count, ob.eq_id[count].xc, count, ob.eq_id[count].yc);
	***/
	//if there are obstacle the count != 0)
	//else is = 0
		return ob;
}

float findRadius(float x0, float y0, float xc, float yc)
{
	float dist;
	float errX, errY, errA; // x, y and average error
	float step;

	dist=ctrlDist(x0, y0, xc, yc);

	//error due to change of direction
	errX = (xc-x0)/dist;
	errY = (yc-y0)/dist;
	errA = (errX + errY)/2;

	//dist *= C_UN;
	//error due to the discance from the target
	step = dist/STEP;
	step *= C_UN;

	return (errA + R_PL + R_SAFETY + step);
}

float ctrlDist(float x, float y, float x2, float y2)
{
	float d;

	d=(x2-x)*(x2-x)+(y2-y)*(y2-y);

	return sqrt(d);
}


double findPoint(double xa, double ya, double xb, double yb, double d, char c)
{
	double tmp = d;

	if((xb-xa) == 0 && c == 'x')
	{
			return xa;
	}
	else
	{
		tmp = atan((yb-ya)/(xb-xa));

		if(c == 'x')
		{
			tmp = cos(tmp);
			tmp *= d;

			//if(ya < yb && xa > xb)
			if(xb < xa){
				if(tmp > 0)
					tmp *= -1;
			}else if(xb > xa){
				if(tmp < 0)
					tmp *=-1;
			}

			return (xa + tmp);
		}
		else
		{
			tmp = sin(tmp);
			tmp *= d;

			//if(ya < yb && xa > xb)
			/////if(ya > yb && xa >= xb)
			if(yb < ya){
				if(tmp > 0)
					tmp *= -1;
			}else if(yb > ya){
				if(tmp < 0)
					tmp *= -1;
			}
		}
	}

	if(yb == ya)
		return ya;
	return (ya + tmp);
}


Point2D PointOnCubicBezier( Point2D* cp, float t )
{
	float   ax, bx, cx;
	float   ay, by, cy;
	float   tSquared, tCubed;
	Point2D result;

/* calculation of coefficients of the polynomial */

	cx = 3.0 * (cp[1].x - cp[0].x);
	bx = 3.0 * (cp[2].x - cp[1].x) - cx;
	ax = cp[3].x - cp[0].x - cx - bx;

	cy = 3.0 * (cp[1].y - cp[0].y);
	by = 3.0 * (cp[2].y - cp[1].y) - cy;
	ay = cp[3].y - cp[0].y - cy - by;

/* point calculation of the curve in relation to t */

	tSquared = t * t;
	tCubed = tSquared * t;

	result.x = (ax * tCubed) + (bx * tSquared) + (cx * t) + cp[0].x;
	result.y = (ay * tCubed) + (by * tSquared) + (cy * t) + cp[0].y;

	return result;
}

void ComputeBezier( Point2D* cp, int numberOfPoints, Point2D* curve )
{
	float   dt;
	int      i;

	/******************************************************
	ocp is an array of 4 elements where:
	cp[0] is the initial point
	cp[1] is the first point of control
	cp[2] is the second point of control
	cp[3] is the final point

	t is the value of the parameter, 0 <= t <= 1
	*******************************************************/

	dt = 1.0 / ( numberOfPoints - 1 );

	for( i = 0; i < numberOfPoints; i++)
		curve[i] = PointOnCubicBezier( cp, i*dt );

	/**
	 * need to draw the curve of matlab or octave *

	printf("x=[");
	for( i = 0; i < numberOfPoints; i++)
	{
		if(i!=numberOfPoints-1)
			printf("%f,",curve[i].x);
		else
			printf("%f];\n",curve[i].x);
	}

	printf("y=[");
	for( i = 0; i < numberOfPoints; i++)
	{
		if(i!=numberOfPoints-1)
			printf("%f,",curve[i].y);
		else
			printf("%f];\nplot(x,y,\"b\",1:8,\"w\")\n\n",curve[i].y);
	}*/
	/**/

	return;
}


boolean inCircle(float x, float y, float xc, float yc, float R)
{
	float dist=ctrlDist(x, y, xc, yc);

	if(dist > R)
		return FALSE;

	return TRUE;
}

void sort(p_ob *array, int begin, int end)
{
	int pivot, l, r;

    if (end > begin) {
       pivot = array[begin].fromMe;
       l = begin + 1;
       r = end+1;
       while(l < r)
          if (array[l].fromMe < pivot)
             l++;
          else {
             r--;
             swap(&(array[l]), &(array[r]));
          }
       l--;
       swap(&(array[begin]), &(array[l]));
       sort(array, begin, l);
       sort(array, r, end);
    }

    return;
}

void swap(p_ob *array, p_ob *array2)
{
	p_ob e;

	e = *array2;
	*array2 = *array;
	*array = e;

	return;
}

allObstacles initNewOb(p_ob ob)
{
	allObstacles newOb;
	int i;

	newOb.eq_id[0] = ob;
	newOb.n_eq = 1;

	for(i=1; i<ALOTOF_PLAYERS; i++)
		newOb.eq_id[i].r = -1;

	return newOb;
}

void checkNewOb(p_ob ob, aplayer_data d, int step, aplayer_data *data,
				int nPlayers, allObstacles *newOb, Point2D start)
{
	int i;
	/* r is the radius of each data[i] */
	float r;

	if(step >= nPlayers)
		return;

	for(i=0; i<nPlayers; i++)
	{
		//if(step == 0)
			r = intRadius(ob, step, d, data[i], start);
		/**else
			r = intRadius(ob, step, d, data[i], start);**/

		if(r != -1)
		{
			if(isNewOb(*newOb, data[i]))
			{
				addIn(newOb, data[i], r);

				checkNewOb(ob, data[i], step+1, data, nPlayers, newOb, start);
			}
		}
	}

	return;
}

float intRadius(p_ob ob, int step, aplayer_data d1, aplayer_data d2, Point2D start)
{
	float r;

	//distance from the centers
	float dCentres;

	r = findRadius(start.x, start.y, d2.x, d2.y);

	if(step == 0)
	{
		dCentres = ctrlDist(ob.xc, ob.yc, d2.x, d2.y);

		if(dCentres < (r + ob.r))
			return r;
	}
	else
	{
		dCentres = ctrlDist(d1.x, d1.y, d2.x, d2.y);

		if(dCentres < (r + findRadius(start.x, start.y, d1.x, d1.y)))
			return r;
	}

	return -1;
}

boolean isNewOb(allObstacles newOb, aplayer_data d)
{
	int i;

	for(i=0; i < newOb.n_eq; i++)
		if(newOb.eq_id[i].xc == d.x)
			if(newOb.eq_id[i].yc == d.y)
				return FALSE;

	return TRUE;
}

void addIn(allObstacles *newOb, aplayer_data d, float r)
{
	(*newOb).eq_id[(*newOb).n_eq].xc = d.x;
	(*newOb).eq_id[(*newOb).n_eq].yc = d.y;
	(*newOb).eq_id[(*newOb).n_eq].r = r;

	(*newOb).n_eq++;
}


void findAllPoints(Point2D start, Point2D end, allObstacles newOb, Point2D *allPoints)
{
	int i;
	Point2D circle[2];
	//distance is necessary to find P0 and P3 (Point of cubic bezier curve)
	Point2D distance[2];
	Point2D p1, p2;
	//d1 and d2 are distance from start point
	float d1, d2, d1tmp, d2tmp;

	//point on the line of trajectory
	Point2D pt1, pt2;

	p1.x = -10000;

	/* allPoints[0] --> first nearest point (on the trajectory line)
	 * that is P0
	 * allPoints[1] --> second point (maximum height) --> P1-P2
	 * allPoints[2] --> third farther point (on the trajectory line) --> P3 */

	/* set the inverse parametre in the bezier points
	 * allPoints[0] need the minimun value and i set the maximum
	 * allPoints[2] need the maximun value and i set the minimum */
	allPoints[0] = end;
	allPoints[2] = start;

	for(i=0; i < newOb.n_eq; i++)
	{
		/* intCircleLine return the two points on the circle that are
		 * on the line perpendicular */
		intCircleLine(newOb.eq_id[i].xc, newOb.eq_id[i].yc, newOb.eq_id[i].r,
						-1, (end.x - start.x)/(end.y - start.y), newOb.eq_id[i].xc,
							newOb.eq_id[i].yc, circle);

		/* i seek the right point that are less distant from start point */
		//in intOtherCircles was passed the parallel line
		if(!intOtherCircles((end.x - start.x), (end.y - start.y),
							circle[0].x, circle[0].y, newOb,
							newOb.eq_id[i].xc, newOb.eq_id[i].yc))
		{
			if(p1.x == -10000)
			{
				p1.x = circle[0].x;
				p1.y = circle[0].y;
			}
			else
			{
				p2.x = circle[0].x;
				p2.y = circle[0].y;
			}
		}
		if(!intOtherCircles((end.x - start.x), (end.y - start.y),
							circle[1].x, circle[1].y, newOb,
							newOb.eq_id[i].xc, newOb.eq_id[i].yc))
		{
			if(p1.x == -10000)
			{
				p1.x = circle[1].x;
				p1.y = circle[1].y;
			}
			else
			{
				p2.x = circle[1].x;
				p2.y = circle[1].y;
			}
		}

		/* distance receives the points of result of intersection
		 * between circle  with the line parallel the line of trajectory*/
		intCircleLine(newOb.eq_id[i].xc, newOb.eq_id[i].yc, newOb.eq_id[i].r,
								 (end.x - start.x), (end.y - start.y),
								 newOb.eq_id[i].xc, newOb.eq_id[i].yc, distance);

		pt1 = int2line((end.x - start.x), (end.y - start.y), start.x,
						start.y, distance[0].x, distance[0].y);

		pt2 = int2line((end.x - start.x), (end.y - start.y), start.x,
						start.y, distance[1].x, distance[1].y);

		d1 = ctrlDist(start.x, start.y, pt1.x, pt1.y);
		d2 = ctrlDist(start.x, start.y, pt2.x, pt2.y);

		d1tmp = ctrlDist(start.x, start.y, allPoints[0].x, allPoints[0].y);
		d2tmp = ctrlDist(start.x, start.y, allPoints[2].x, allPoints[2].y);

		if((d1 <= d1tmp) || (d2 <= d1tmp))//add =
		{
			if(d1 <= d1tmp && d1 <= d2)
				allPoints[0] = pt1;
			else
				allPoints[0] = pt2;
		}

		if((d1 >= d2tmp) || (d2 >= d2tmp))//add =
		{
			if(d1 >= d2tmp && d1 >= d2)
				allPoints[2] = pt1;
			else
				allPoints[2] = pt2;
		}
	}

	d1 = ctrlDist(start.x, start.y, p1.x, p1.y);
	d2 = ctrlDist(start.x, start.y, p2.x, p2.y);

	if(d1 <= d2)
		allPoints[1] = p1;
	else
		allPoints[1] = p2;

	return;
}


void intCircleLine(float xc, float yc, float R, float alpha,
						float beta, float x0, float y0, Point2D *circle)
{
	float t, A, B, C, delta;

	A = pow((alpha),2) + pow((beta),2);
	B = (2*x0-2*xc)*(alpha) + (beta)*(2*y0-2*yc);
	C = x0*x0 + xc*xc - 2*x0*xc + y0*y0 + yc*yc -2*y0*yc - R*R;

	delta = B*B -4*A*C;

	// if delta < 0 there are none intersection
	// if delta = 0 the safety radius allows us to move without collision
	// if delta > 0 this circle is an obstacle on the line

	if(delta > 0)
	{
		/***printf("R = %f\n", R);***/

		t = -1*(B + sqrt(delta))/(2*A);

		circle[0].x = x0 + ((alpha)*t);
		circle[0].y = y0 + ((beta)*t);

		t = -1*(B - sqrt(delta))/(2*A);

		circle[1].x = x0 + ((alpha)*t);
		circle[1].y = y0 + ((beta)*t);

		return;
	}
}


boolean intOtherCircles(float alpha, float beta, float x0, float y0,
						allObstacles newOb, float xc, float yc)
{
	int i;
	float A, B, C, delta;

	//i've the parallel line and now check the intersection with all the circles
	for(i=0; i < newOb.n_eq; i++)
	{
		delta = -1;
		if(newOb.eq_id[i].xc != xc && newOb.eq_id[i].yc != yc)
		{
			A = pow((alpha),2) + pow((beta),2);
			B = (2*x0-2*newOb.eq_id[i].xc)*(alpha) + (beta)*(2*y0-2*newOb.eq_id[i].yc);
			C = x0*x0 + newOb.eq_id[i].xc*newOb.eq_id[i].xc - 2*x0*newOb.eq_id[i].xc
			+ y0*y0 + newOb.eq_id[i].yc*newOb.eq_id[i].yc -2*y0*newOb.eq_id[i].yc
			- newOb.eq_id[i].r*newOb.eq_id[i].r;

			delta = B*B -4*A*C;
		}
		// if delta < 0 there are none intersection
		// if delta = 0 the safety radius allows us to move without collision
		// if delta > 0 this circle is an obstacle on the line

		if(delta >= 0)
			return TRUE;
	}

	return FALSE;
}

Point2D int2line(float alpha, float beta, float a, float b, float xc, float yc)
{
	Point2D d;
	float c;

	c = (-1*(alpha*alpha)/(beta));

	d.x = (c*xc - alpha*yc - beta*a + alpha*b)/(c - beta);

	d.y = (alpha/beta)*xc - (alpha/beta)*d.x + yc;

	return d;
}

void findBezierPoints(Point2D* p, Point2D start, Point2D end, allObstacles newOb, Point2D *b)
{
	//Point2D b[4];  bezier points
	Point2D t[2]; // temp points
	Point2D tmp;
	float d1, d2;

	//Search begins first point

	intCircleLine(p[0].x, p[0].y, RADIUS_DOWN_BEZIER*C_BEZIER(newOb.n_eq)*newOb.n_eq,
					  (end.x-start.x), (end.y - start.y), start.x, start.y, t);

	d1 = ctrlDist(start.x, start.y, t[0].x, t[0].y);
	d2 = ctrlDist(start.x, start.y, t[1].x, t[1].y);

	if(d1 < d2)
		tmp = t[0];
	else
		tmp = t[1];

	//to find the first point P0
	if(end.x > start.x)
	{
		if(tmp.x < start.x)
			b[0] = start;
		else
			b[0] = tmp;
	}
	else if(end.x == start.x)
	{
		if(end.y > start.y)
		{
			if(tmp.y < start.y)
				b[0] = start;
			else
				b[0] = tmp;
		}
		else
		{
			if(tmp.y > start.y)
				b[0] = start;
			else
				b[0] = tmp;
		}
	}
	else //if(end.x < start.x)
	{
		if(tmp.x > start.x)
			b[0] = start;
		else
			b[0] = tmp;
	}

	//end of the first search point
	//Search begins fourth point P3

	intCircleLine(p[2].x, p[2].y, RADIUS_DOWN_BEZIER*C_BEZIER(newOb.n_eq)*newOb.n_eq,
					  (end.x-start.x), (end.y - start.y), start.x, start.y, t);

	d1 = ctrlDist(start.x, start.y, t[0].x, t[0].y);
	d2 = ctrlDist(start.x, start.y, t[1].x, t[1].y);

	if(d1 > d2)
		tmp = t[0];
	else
		tmp = t[1];

	//to find the first point P3
	if(end.x > start.x)
	{
		if(tmp.x > end.x)
			b[3] = end;
		else
			b[3] = tmp;
	}
	else if(end.x == start.x)
	{
		if(end.y > start.y)
		{
			if(tmp.y > end.y)
				b[3] = end;
			else
				b[3] = tmp;
		}
		else
		{
			if(tmp.y < end.y)
				b[3] = end;
			else
				b[3] = tmp;
		}
	}
	else //if(end.x < start.x)
	{
		if(tmp.x < end.x)
			b[3] = end;
		else
			b[3] = tmp;
	}

	//end of the fourth search point
	//Search begins fourth point P1 and P2

	intCircleLine(p[1].x, p[1].y, RADIUS_UP_BEZIER*C_BEZIER(newOb.n_eq)*newOb.n_eq,
					  -1, (end.x - start.x)/(end.y - start.y), p[1].x, p[1].y, t);

	d1 = ctrlDist(start.x, start.y, t[0].x, t[0].y);
	d2 = ctrlDist(start.x, start.y, t[1].x, t[1].y);

	if(d1 > d2)
		tmp = t[0];
	else
		tmp = t[1];

	intCircleLine(tmp.x, tmp.y, RADIUS_UP_BEZIER*C_BEZIER(newOb.n_eq)*newOb.n_eq,
					   (end.x - start.x), (end.y - start.y), tmp.x, tmp.y, t);

	d1 = ctrlDist(start.x, start.y, t[0].x, t[0].y);
	d2 = ctrlDist(start.x, start.y, t[1].x, t[1].y);

	if(d1 > d2)
	{
		b[1] = t[1];
		b[2] = t[0];
	}
	else
	{
		b[1] = t[0];
		b[2] = t[1];
	}

	return;
}

Point2D becomeCurved(server_data *server, Point2D *curve, aplayer_data *data,
				int nPlayers, allObstacles newOb, double *ang, boolean  modify_ang)
{
	int i, f;//f is the fromTo function flag
	double ang_tmp;
	Point2D del;//variable that need to exit from the program
	//double aTmp = *ang;

	for(i=0; i < CurvePoints-1; i++)
	{
		/***printf("id=%d  x=%f  y=%f\n",i+1,curve[i].x,curve[i].y);***/
		system("sleep 0.1");

		if((i + BEZIER_STEP) <= CurvePoints)
		{
			if(pointInCircle(curve[i+BEZIER_STEP], nPlayers, data))
				return curve[i];
		}

		if(curve[i+1].x-curve[i].x == 0)
		{
			if(curve[i+1].y == curve[i].y)
				ang_tmp = 0;
			else if(curve[i+1].y > curve[i].y)
				ang_tmp = 90;
			else //if(curve[i+1].y < curve[i].y)
				ang_tmp = -90;
		}
		else
		{
			ang_tmp = atan((curve[i+1].y-curve[i].y)/(curve[i+1].x-curve[i].x));
			ang_tmp = ang_tmp*180/M_PI;
		}

		/***printf("a_tmp=%f\n\n",ang_tmp);***/

		if(curve[CurvePoints-1].x > curve[0].x &&
					curve[CurvePoints-1].y > curve[0].y)
			if(fabs(curve[i].x) < fabs(curve[i+1].x)) //&& curve[i].y < curve[i+1].y)
						if(ang_tmp < 0)
							ang_tmp += 180;

		if(curve[CurvePoints-1].x > curve[0].x &&
					curve[CurvePoints-1].y < curve[0].y)
				if(fabs(curve[i].x) < fabs(curve[i+1].x)) //&& curve[i].y < curve[i+1].y)
						if(ang_tmp > 0)
							ang_tmp -= 180;

		if(curve[CurvePoints-1].x < curve[0].x &&
					curve[CurvePoints-1].y < curve[0].y)
				if(fabs(curve[i].x) < fabs(curve[i+1].x)) //&& y < y2)
						if(ang_tmp > 0)
							ang_tmp -= 180;

		if(curve[CurvePoints-1].x < curve[0].x &&
					curve[CurvePoints-1].y > curve[0].y)
				if(fabs(curve[i].x) < fabs(curve[i+1].x)) //&& y < y2)
				//{
					if(ang_tmp < 0)
						ang_tmp += 180;
				//	else
				//		ang_tmp *= -1;
				//}

		f=fromTo(server, (double) curve[i].x, (double) curve[i].y, *ang,
				(double) curve[i+1].x, (double) curve[i+1].y, ang_tmp, modify_ang);

		if(f)
		{
			del.x = -12345;
			del.y = -12345;
			return del;
		}

		*ang = ang_tmp;
		a = ang_tmp;
		//printf("angle %d = %f\n\n",i+1,*ang);
	}

	//*ang = aTmp;

	//printf("angle total before to return=%f\n\n\n",*ang);
	return curve[CurvePoints-1];
}

boolean pointInCircle(Point2D p, int nPlayers, aplayer_data *data)
{
	int i;
	float r;

	for(i=0; i<nPlayers; i++)
	{
		r = findRadius(p.x, p.y, data[i].x, data[i].y);

		if(inCircle(p.x, p.y, data[i].x, data[i].y, r))
			return TRUE;
	}

	return FALSE;
}

void insertNewOb(allObstacles ob, allObstacles *newOb)
{
	int i;
	aplayer_data data;

	for(i=1; i<ob.n_eq; i++)
	{
		data.x = ob.eq_id[i].xc;
		data.y = ob.eq_id[i].yc;

		if(ctrlDist(ob.eq_id[i-1].xc, ob.eq_id[i-1].yc, ob.eq_id[i].xc, ob.eq_id[i].yc)
			< THRESHOLD)
		{
			if(isNewOb(*newOb, data))
				addIn(newOb, data, ob.eq_id[i].r);
		}
	}
}

boolean tolerance_angle(double ang, double ang2)
{
	double t1,t2;

	t1 = ang - THRESHOLD_ANGLE;
	t2 = ang + THRESHOLD_ANGLE;

	/* there isn't an angle smaller than -180 and greater than 180 */
	if(t1 < -180)
		t1 = t1 + 180 + 180;
	if(t2 > 180)
		t2 = t2 - 180 - 180;

	/*if(last_act=='t');
		return FALSE;*/
	
		
	if(t1 <= ang2 && ang2 <= t2)
		return TRUE;

	return FALSE;
}













