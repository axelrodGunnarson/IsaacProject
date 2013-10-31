#ifndef TRAJECTORY_H
#define TRAJECTORY_H
/*
 *  trajectory.h
 *  Isaac_Agent
 *
 *  Created by anthos89.
 *  Copyright 2011 __YourCompanyName__. All rights reserved.
 *
 */

#include "blackboard.h"

#include <math.h>

#define THRESHOLD 15.5
#define THRESHOLD_ANGLE 60 //threshold angle in which a turn can be valid.. it's not so clear, uh?
#define THRESHOLD_TURN 20 // max TURN can be done in a cycle
#define STEP 5
#define R_PL 1//0.4//0.3 //RADIUS PLAYERS
#define R_SAFETY 0.65 //range safety
#define C_UN 0.3333333333 //coefficient of uncertainty
/* bezier cofficient
 * bezier points increases with the distance depending on
 * the number of obstacles */
#define C_BEZIER(x) (x==1 ? 2.5 : 1.15)//1.15//1.5
/* RADIUS_DOWN_BEZIER is for the circle on the trajectory line */
#define RADIUS_DOWN_BEZIER 2.5//0.5
#define RADIUS_UP_BEZIER 1//3.5
#define CurvePoints 40
/* need to see if there are same obstacles in my path */
#define BEZIER_STEP 6

/* DIGIT_DECIMAL needs to take only the first 4 decimal digit of a float */
#define DIGIT_DECIMAL(x) ((int)(x*100))/((double)100)

//identifies the obstacle position
typedef struct{
			//the centre is identified by xc and yc
			float xc;
			float yc;
			//the radius
			float r;
			//distance to the obstacle
			float fromMe;
			/* the first intersection is identified by x1 and y1 */
			float x1;
			float y1;
			/* the first intersection is identified by x2 and y2 */
			float x2;
			float y2;
		}p_ob;

typedef struct{
			//equation identifier
			p_ob eq_id[ALOTOF_PLAYERS];
			//number of equations
			int n_eq;
		}allObstacles;

typedef struct{
			float x;
			float y;
		}Point2D;



void trajectory_thread(server_data *server);


/* Initial function of thread */
/* move with dash from one point to another point avoiding obstacles*/
void trajectory(server_data *server);

/**/
double setTurn(double a, double ang);

/* sets the maximum allowable value for power and direction */
void modify_kick(double *power, double *direction);

/* move with dash from one point to another point*/
int fromTo(server_data *server, double x, double y, double ang,
			double x2, double y2, double ang2, boolean modify_ang);

/* function that do getAbsPlayers and look if there are
 * intersections between the equation of
 * my line and circumference of the obstacles
 * and put the in nPlayers the real number of players in the field */
allObstacles ctrlObstacles(double x0, double y0, double x1, double y1, int *nPlayers);

/* find radius of the circle */
float findRadius(float x0, float y0, float xc, float yc);

/* control distance between 2 points */
float ctrlDist(float x, float y, float x2, float y2);

/* find a point (xc,yc) on my line away d starting from (xa,ya)
 * for to calculate xc, must be equal to 'x'
 * for to calculate yc, must be equal to 'y' */
double findPoint(double xa, double ya, double xb, double yb, double d, char c);




/******************************************************
* Code to create a cubic Bezier curve
*******************************************************/
Point2D PointOnCubicBezier( Point2D* cp, float t );

/******************************************************************************
ComputeBezier fills an array of structures Point2D with the points of the curve
generated by the checkpoints cp. The caller must allocate memory
which is sufficient for the result * <sizeof(Point2D) numeroDiPunti>
******************************************************************************/
void ComputeBezier( Point2D* cp, int numberOfPoints, Point2D* curve );

/* see if the points x and y are inside the circle
 * centered at xc and yc with radius R*/
boolean inCircle(float x, float y, float xc, float yc, float R);

/* make quicksort ordering the array to increasing distance from obstacles
 * where begin is 0 and end is the number of elments minus one*/
void sort(p_ob *array, int begin, int end);

/* change the position of the two elements of array
 * passed as parameters */
void swap(p_ob *array, p_ob *array2);

/* control if near the obstacle on the path there is another
 * obstacle that prevents the passage */
/* ob        --> is the (initial) obstacle on my trajectory
 * d         --> is the player connected to the initial obstacle
 * step      --> is the step of the recursion
 * data      --> is the array with all the players
 * nPlayers  --> nuber of players in the field
 * newOb     --> array with the new obstacle
 * start     --> initial point of my trajectory */
void checkNewOb(p_ob ob, aplayer_data d, int step, aplayer_data *data,
				int nPlayers, allObstacles *newOb, Point2D start);

/* initializes an array of new obstacles and put in the first
 * element of the array ob */
allObstacles initNewOb(p_ob ob);

/* control if there is an intersection between ob and d2
 * if the step is equal 0 or otherwise check an intersection
 * between d1 and d2. In any case return the radius of d2 */
float intRadius(p_ob ob, int step, aplayer_data d1, aplayer_data d2, Point2D start);

/* find the points that will be examined to find the Bezier curve */
void findAllPoints(Point2D start, Point2D end, allObstacles newOb, Point2D *allPoints);

/* find the intersection between circle and parametric line */
void intCircleLine(float xc, float yc, float R, float alpha,
						float beta, float x0, float y0, Point2D *circle);

/* checks whether a straight line (parallel to the line of trajectory and passing
 * through (x0, y0)) intersects the circumference of the players */
boolean intOtherCircles(float alpha, float beta, float x0, float y0,
						allObstacles newOb, float xc, float yc);

/* intersection between two parametric PERPENDICULAR line */
Point2D int2line(float alpha, float beta, float a, float b, float xc, float yc);

/* find the correct bezezier points */
void findBezierPoints(Point2D* p, Point2D start, Point2D end, allObstacles newOb, Point2D *b);

/* move the robot towards the bezier trajectory */
Point2D becomeCurved(server_data *server, Point2D *curve, aplayer_data *data,
			int nPlayers, allObstacles newOb, double *ang, boolean modify_ang);

/* check if point p is into one of all the circle on the field */
boolean pointInCircle(Point2D p, int nPlayers, aplayer_data *data);

/* find if d is in newOb */
boolean isNewOb(allObstacles newOb, aplayer_data d);

/* put d into newOb */
void addIn(allObstacles *newOb, aplayer_data d, float r);

/* insert in newOb the ob that distance is less than THRESHOLD from
 * other obstacles on the trajectory */
void insertNewOb(allObstacles ob, allObstacles *newOb);

/* tolerance_angle see if ang, with THRESHOLD_ANGLE, is equal to ang2 */
boolean tolerance_angle(double ang, double ang2);

#endif







