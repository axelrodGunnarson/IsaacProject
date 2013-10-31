#ifndef RULES_H
#define RULES_H

/*
 *  rules.h
 *  Isaac_Agent
 *
 *  Created by erix on 04/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

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

#define MAX_N_RULES 1024
#define MAX_RULE_LENGTH 255
#define N_CONDITIONS 2

//DA MODIFICARE????
#define N_RULES 30

//action definition
#define A_GOTO_BALL 0
#define A_GOTO_MATEx 1x
#define A_GOTO_OPPONENTx 2x
#define A_GOTO_TEAMGOAL 3
#define A_GOTO_OPPONENTGOAL 4
//....

//condition definition
#define GOT_BALL 0
#define NEAR_OPPONENT_GOAL 1




typedef struct Rule{
	int conditions[N_CONDITIONS];
	int action;
	int valid;
} rule;

typedef struct Rules{
	rule action[PLAY_TIMES][MAX_N_RULES];

} rules;


int parseRules(rules *it_rules, FILE *file);
int parseRule(char *string, rule *location);
void initRules(rules *it_rules);
void printRules(rules *it_rules);
void fprintRules(rules *it_rules, FILE *dest);



#endif
