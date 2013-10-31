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


#include "../include/rules.h"
#include "../include/errlib.h"


#define GOT_BALL_THRES 1


//to be checked
int parseRules(rules *it_rules, FILE *file){
	char new[MAX_RULE_LENGTH+1];
	int playmode=0, rule_position=0;



	//Read a new rule from file, until it ends
	while(fgets(new, MAX_RULE_LENGTH, file) != NULL){
		if(new[0]=='P'){
			//this line starts a new playmode section
			sscanf(new,"P%i\n",&playmode);
			rule_position=0;
		}else{
			//parse the specific rule!
			//printf("New rule in position: %i %i\n",playmode,rule_position);
			parseRule(new, &(it_rules->action[playmode][rule_position++]));
		}
	}

	return 0;
}




//to be checked
int parseRule(char *string, rule *location){
	int i=0;
	char val=0;
	int stop=0;

	//DEBUG
	//printf("Parsing: %s\n",string);

	//condition parsing
	for(i=0;!stop && i<N_CONDITIONS;i++){
		val=string[i];
		if(val!='-'){
			location->conditions[i]=(int)(val-'0'); // val-'0' is to cast from char to int!
			//printf("condition: %i\n",location->conditions[i]);
		}
		else
			stop++;
	}

	//action parsing
	//sscanf(string+i,"->%i\n",&val);
	val=string[i+1];
	location->action=(int)(val-'0'); // as above

	location->valid=1;

	//DEBUG
	/*printf("Wrote line: ");
	for(i=0;i<N_CONDITIONS;i++)
		printf("%i",location->conditions[i]);

	printf(" - %i\n", location->action);
	*/
	return 0;
}


void initRules(rules *it_rules){
	int i,j;

	for(i=0;i<PLAY_TIMES;i++)
		for(j=0;j<MAX_N_RULES;j++){
			it_rules->action[i][j].valid=-1;
			memset(&(it_rules->action[i][j]),0,sizeof(int)*N_CONDITIONS);
			it_rules->action[i][j].action=-1;
		}
}

void printRules(rules *it_rules){
	int i=0;
	printf("PrintRules:\n");

	for(i=0;i<PLAY_TIMES;i++){
		int j=0;
		printf("**Playmode: %i\n",i);
		while(it_rules->action[i][j].valid!=-1){
			int v=0;
			printf("%i",it_rules->action[i][j].conditions[v]);
			for(v=1;v<N_CONDITIONS;v++)
				printf(",%i",it_rules->action[i][j].conditions[v]);
			printf("-> %i\n",it_rules->action[i][j].action);
			j++;
		}
	}
}


void fprintRules(rules *it_rules, FILE *dest){
	int i=0,v=0;
	//printf("PrintRules:\n");

	for(i=0;i<PLAY_TIMES;i++){
		int j=0;
		fprintf(dest,"P%i\n",i);
		while(it_rules->action[i][j].valid!=-1){
			/*int v=0;
			fprintf(dest,"%i",it_rules->action[i][j].conditions[v]);*/
			for(v=0;v<N_CONDITIONS;v++)
				fprintf(dest,"%i",it_rules->action[i][j].conditions[v]);
			fprintf(dest,"-%i\n",it_rules->action[i][j].action);
			j++;
		}
	}
}






