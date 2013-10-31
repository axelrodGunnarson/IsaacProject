#ifndef AI_H
#define AI_H



#include "rules.h"
#include "sensor.h"

int aiThread();
int valuateSituation(int *situation);
int checkGotBall();
int checkNearOpponentGoal();
double distance(double x1, double y1, double x2, double y2);



#endif
