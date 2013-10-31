#include <stdio.h>
#include <stdlib.h>
#include "../include/localization.h"
#include "../include/errlib.h"

int riga1 [11] = {FLAG_T_L_50,FLAG_T_L_40,FLAG_T_L_30,FLAG_T_L_20,FLAG_T_L_10,FLAG_T_0,FLAG_T_R_10,FLAG_T_R_20,FLAG_T_R_30,FLAG_T_R_40,FLAG_T_R_50};
int riga2 [3] = {FLAG_L_T,FLAG_C_T,FLAG_R_T};
int riga3 [2] = {FLAG_L_T_30, FLAG_R_T_30};
int riga4 [4] = {FLAG_L_T_20,FLAG_P_L_T,FLAG_P_R_T,FLAG_R_T_20};
int riga5 [2] = {FLAG_L_T_10,FLAG_R_T_10};
int riga6 [2] = {FLAG_G_L_T,FLAG_G_R_T};
int riga7 [7] = {FLAG_L_0,FLAG_G_L,FLAG_P_L_C,FLAG_C,FLAG_P_R_C,FLAG_G_R,FLAG_R_0};
int riga8 [2] = {FLAG_G_L_B,FLAG_G_R_B};
int riga9 [2] = {FLAG_L_B_10,FLAG_R_B_10};
int riga10 [4] = {FLAG_L_B_20,FLAG_P_L_B,FLAG_P_R_B,FLAG_R_B_20};
int riga11 [2] = {FLAG_L_B_30,FLAG_R_B_30};
int riga12 [3] = {FLAG_L_B,FLAG_C_B,FLAG_R_B};
int riga13 [11] = {FLAG_B_L_50,FLAG_B_L_40,FLAG_B_L_30,FLAG_B_L_20,FLAG_B_L_10,FLAG_B_0,FLAG_B_R_10,FLAG_B_R_20,FLAG_B_R_30,FLAG_B_R_40,FLAG_B_R_50};
int colonna1 [7] = {FLAG_L_T_30,FLAG_L_T_20,FLAG_L_T_10,FLAG_L_0,FLAG_L_B_10,FLAG_L_B_20,FLAG_L_B_30};
int colonna2 [5] = {FLAG_L_T,FLAG_G_L_T,FLAG_G_L,FLAG_G_L_B,FLAG_L_B};
int colonna3 [2] = {FLAG_T_L_50,FLAG_B_L_50};
int colonna4 [2] = {FLAG_T_L_40,FLAG_B_L_40};
int colonna5 [3] = {FLAG_P_L_T,FLAG_P_L_C,FLAG_P_L_B};
int colonna6 [2] = {FLAG_T_L_30,FLAG_B_L_30};
int colonna7 [2] = {FLAG_T_L_20,FLAG_B_L_20};
int colonna8 [2] = {FLAG_T_L_10,FLAG_B_L_10};
int colonna9 [5] = {FLAG_T_0,FLAG_C_T,FLAG_C,FLAG_C_B,FLAG_B_0};
int colonna10 [2] = {FLAG_T_R_10,FLAG_B_R_10};
int colonna11 [2] = {FLAG_T_R_20,FLAG_B_R_20};
int colonna12 [2] = {FLAG_T_R_30,FLAG_B_R_30};
int colonna13 [3] = {FLAG_P_R_T,FLAG_P_R_C,FLAG_P_R_B};
int colonna14 [2] = {FLAG_T_R_40,FLAG_B_R_40};
int colonna15 [2] = {FLAG_T_R_50,FLAG_B_R_50};
int colonna16 [5] = {FLAG_R_T,FLAG_G_R_T,FLAG_G_R,FLAG_G_R_B,FLAG_R_B};
int colonna17 [7] = {FLAG_R_T_30,FLAG_R_T_20,FLAG_R_T_10,FLAG_R_0,FLAG_R_B_10,FLAG_R_B_20,FLAG_R_B_30};

void init () {
int i=0;

//printf("Dentro init - Indirizzo di vett_base_righe: %p TOTALE_RIGHE: %d\n",vett_base_righe, TOTALE_RIGHE);
//printf("Dentro init - Indirizzo di vett_base_colonne: %p TOTALE_COLONNE: %d\n",vett_base_colonne, TOTALE_COLONNE);
//printf("Dentro init - Indirizzo di vett_offset: %p N_FLAGS: %d\n",vett_offset, N_FLAGS);
//funzione che riempie tutto il vett di base con i vari elementi; può essere solo sequenziale, non a ciclo perchè le dimensioni sono diverse
vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali= 11;
vett_base_righe[i].vett = riga1;
i++;
vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali= 3;
vett_base_righe[i].vett = riga2;
i++;
vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali=2;
vett_base_righe[i].vett = riga3;
i++;
/*vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali= 2;
vett_base_righe[i].vett = riga3;
i++;*/
vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali= 4;
vett_base_righe[i].vett = riga4;
i++;
vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali= 2;
vett_base_righe[i].vett = riga5;
i++;
vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali= 2;
vett_base_righe[i].vett = riga6;
i++;
vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali= 7;
vett_base_righe[i].vett = riga7;
i++;
vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali= 2;
vett_base_righe[i].vett = riga8;
i++;
vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali= 2;
vett_base_righe[i].vett = riga9;
i++;
vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali= 4;
vett_base_righe[i].vett = riga10;
i++;
vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali= 2;
vett_base_righe[i].vett = riga11;
i++;
vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali= 3;
vett_base_righe[i].vett = riga12;
i++;
vett_base_righe[i].presenti=0;
vett_base_righe[i].num_pali= 11;
vett_base_righe[i].vett = riga13;
i=0;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 7;
vett_base_colonne[i].vett = colonna1;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 5;
vett_base_colonne[i].vett = colonna2;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 2;
vett_base_colonne[i].vett = colonna3;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 2;
vett_base_colonne[i].vett = colonna4;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 3;
vett_base_colonne[i].vett = colonna5;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 2;
vett_base_colonne[i].vett = colonna6;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 2;
vett_base_colonne[i].vett = colonna7;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 2;
vett_base_colonne[i].vett = colonna8;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 5;
vett_base_colonne[i].vett = colonna9;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 2;
vett_base_colonne[i].vett = colonna10;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 2;
vett_base_colonne[i].vett = colonna11;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 2;
vett_base_colonne[i].vett = colonna12;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 3;
vett_base_colonne[i].vett = colonna13;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 2;
vett_base_colonne[i].vett = colonna14;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 2;
vett_base_colonne[i].vett = colonna15;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 5;
vett_base_colonne[i].vett = colonna16;
i++;
vett_base_colonne[i].presenti=0;
vett_base_colonne[i].num_pali= 7;
vett_base_colonne[i].vett = colonna17;

//in seguito riempio il vettore di offset

i=0;
vett_offset[FLAG_T_L_50].offset_x = 7.5;
vett_offset[FLAG_T_L_50].offset_y = 78;
i++;
vett_offset[FLAG_T_L_40].offset_x = 17.5;
vett_offset[FLAG_T_L_40].offset_y = 78;
i++;
vett_offset[FLAG_T_L_30].offset_x = 27.5;
vett_offset[FLAG_T_L_30].offset_y = 78;
i++;
vett_offset[FLAG_T_L_20].offset_x = 37.5;
vett_offset[FLAG_T_L_20].offset_y = 78;
i++;
vett_offset[FLAG_T_L_10].offset_x = 47.5;
vett_offset[FLAG_T_L_10].offset_y = 78;
i++;
vett_offset[FLAG_T_0].offset_x = 57.5;
vett_offset[FLAG_T_0].offset_y = 78;
i++;
vett_offset[FLAG_T_R_10].offset_x = 67.5;
vett_offset[FLAG_T_R_10].offset_y = 78;
i++;
vett_offset[FLAG_T_R_20].offset_x = 77.5;
vett_offset[FLAG_T_R_20].offset_y = 78;
i++;
vett_offset[FLAG_T_R_30].offset_x = 87.5;
vett_offset[FLAG_T_R_30].offset_y = 78;
i++;
vett_offset[FLAG_T_R_40].offset_x = 97.5;
vett_offset[FLAG_T_R_40].offset_y = 78;
i++;
vett_offset[FLAG_T_R_50].offset_x = 107.5;
vett_offset[FLAG_T_R_50].offset_y = 78;
i++; //11
vett_offset[FLAG_R_T_30].offset_x = 115;
vett_offset[FLAG_R_T_30].offset_y = 69;
i++;
vett_offset[FLAG_R_T_20].offset_x = 115;
vett_offset[FLAG_R_T_20].offset_y = 59;
i++;
vett_offset[FLAG_R_T_10].offset_x = 115;
vett_offset[FLAG_R_T_10].offset_y = 49;
i++;
vett_offset[FLAG_R_0].offset_x = 115;
vett_offset[FLAG_R_0].offset_y = 39;
i++;
vett_offset[FLAG_R_B_10].offset_x = 115;
vett_offset[FLAG_R_B_10].offset_y = 29;
i++;
vett_offset[FLAG_R_B_20].offset_x = 115;
vett_offset[FLAG_R_B_20].offset_y = 19;
i++;
vett_offset[FLAG_R_B_30].offset_x = 115;
vett_offset[FLAG_R_B_30].offset_y = 9;
i++; //18
vett_offset[FLAG_B_R_50].offset_x = 107.5;
vett_offset[FLAG_B_R_50].offset_y = 0;
i++;
vett_offset[FLAG_B_R_40].offset_x = 97.5;
vett_offset[FLAG_B_R_40].offset_y = 0;
i++;
vett_offset[FLAG_B_R_30].offset_x = 87.5;
vett_offset[FLAG_B_R_30].offset_y = 0;
i++;
vett_offset[FLAG_B_R_20].offset_x = 77.5;
vett_offset[FLAG_B_R_20].offset_y = 0;
i++;
vett_offset[FLAG_B_R_10].offset_x = 67.5;
vett_offset[FLAG_B_R_10].offset_y = 0;
i++;
vett_offset[FLAG_B_0].offset_x = 57.5;
vett_offset[FLAG_B_0].offset_y = 0;
i++;
vett_offset[FLAG_B_L_10].offset_x = 47.5;
vett_offset[FLAG_B_L_10].offset_y = 0;
i++;
vett_offset[FLAG_B_L_20].offset_x = 37.5;
vett_offset[FLAG_B_L_20].offset_y = 0;
i++;
vett_offset[FLAG_B_L_30].offset_x = 27.5;
vett_offset[FLAG_B_L_30].offset_y = 0;
i++;
vett_offset[FLAG_B_L_40].offset_x = 17.5;
vett_offset[FLAG_B_L_40].offset_y = 0;
i++;
vett_offset[FLAG_B_L_50].offset_x = 7.5;
vett_offset[FLAG_B_L_50].offset_y = 0;
i++; //29
vett_offset[FLAG_L_B_30].offset_x = 0;
vett_offset[FLAG_L_B_30].offset_y = 9;
i++;
vett_offset[FLAG_L_B_20].offset_x = 0;
vett_offset[FLAG_L_B_20].offset_y = 19;
i++;
vett_offset[FLAG_L_B_10].offset_x = 0;
vett_offset[FLAG_L_B_10].offset_y = 29;
i++;
vett_offset[FLAG_L_0].offset_x = 0;
vett_offset[FLAG_L_0].offset_y = 39;
i++;
vett_offset[FLAG_L_T_10].offset_x = 0;
vett_offset[FLAG_L_T_10].offset_y = 49;
i++;
vett_offset[FLAG_L_T_20].offset_x = 0;
vett_offset[FLAG_L_T_20].offset_y = 59;
i++;
vett_offset[FLAG_L_T_30].offset_x = 0;
vett_offset[FLAG_L_T_30].offset_y = 69;
i++; //36
vett_offset[FLAG_L_T].offset_x = 5;
vett_offset[FLAG_L_T].offset_y = 73;
i++;
vett_offset[FLAG_C_T].offset_x = 57.5;
vett_offset[FLAG_C_T].offset_y = 73;
i++;
vett_offset[FLAG_R_T].offset_x = 110;
vett_offset[FLAG_R_T].offset_y = 73;
i++; //39
vett_offset[FLAG_G_R_T].offset_x = 110;
vett_offset[FLAG_G_R_T].offset_y = 46.01;
i++;
vett_offset[FLAG_G_R].offset_x = 110;
vett_offset[FLAG_G_R].offset_y = 39;
i++;
vett_offset[FLAG_G_R_B].offset_x = 110;
vett_offset[FLAG_G_R_B].offset_y = 31.99;
i++; //42
vett_offset[FLAG_R_B].offset_x = 110;
vett_offset[FLAG_R_B].offset_y = 5;
i++;
vett_offset[FLAG_C_B].offset_x = 57.5;
vett_offset[FLAG_C_B].offset_y = 5;
i++; //44
vett_offset[FLAG_L_B].offset_x = 5;
vett_offset[FLAG_L_B].offset_y = 5;
i++;
vett_offset[FLAG_G_L_B].offset_x = 5;
vett_offset[FLAG_G_L_B].offset_y = 31.99;
i++;
vett_offset[FLAG_G_L].offset_x = 5;
vett_offset[FLAG_G_L].offset_y = 39;
i++;
vett_offset[FLAG_G_L_T].offset_x = 5;
vett_offset[FLAG_G_L_T].offset_y = 46.01;
i++; //48
vett_offset[FLAG_P_L_T].offset_x = 21.5;
vett_offset[FLAG_P_L_T].offset_y = 59.16;
i++;
vett_offset[FLAG_P_R_T].offset_x = 93.5;
vett_offset[FLAG_P_R_T].offset_y = 59.16;
i++;
vett_offset[FLAG_P_R_C].offset_x = 93.5;
vett_offset[FLAG_P_R_C].offset_y = 39;
i++; //51
vett_offset[FLAG_P_R_B].offset_x = 93.5;
vett_offset[FLAG_P_R_B].offset_y = 18.84;
i++;
vett_offset[FLAG_P_L_B].offset_x = 21.5;
vett_offset[FLAG_P_L_B].offset_y = 18.84;
i++;
vett_offset[FLAG_P_L_C].offset_x = 21.5;
vett_offset[FLAG_P_L_C].offset_y = 39;
i++;
vett_offset[FLAG_C].offset_x = 57.5;
vett_offset[FLAG_C].offset_y = 39;

}

/*
void print() {

int num_players;
body_data io_a = getBody(&blackboard);
printf ("\n stampo dati del player:");
printf ("\n x: %f y: %f head_angle: %f", io_a.x, io_a.y, io_a.body_angle);
rball_data temp_rb = getRelBall(&blackboard);
if (temp_rb.time != -1) {
	printf ("\n stampo dati relativi palla:");
	printf ("\n distance: %f direction: %f dir_change: %f dist_change: %f", temp_rb.distance, temp_rb.direction, temp_rb.dir_change, temp_rb.dist_change);
	}
else
	printf("\n Non vedo la palla");
printf ("\n stampo pali visti relativi:");
int i;
for (i=0; i< N_FLAGS; i++) {
	rflag_data temp_rf = getRelFlag(i, &blackboard);
	if (temp_rf.time != -1)
		printf ("\n palo numero %d: time: %d, distance: %f direction: %f", i, temp_rf.time, temp_rf.distance, temp_rf.direction);
	}
printf ("\n stampo giocatori visti relativi:");
rplayer_data * vettrelplayers = (rplayer_data *) malloc (ALOTOF_PLAYERS * sizeof (rplayer_data));
getRelPlayers (&blackboard, &num_players, vettrelplayers);
for (i=0; i<num_players; i++)
	if (vettrelplayers[i].time != -1)
		printf("\n distance: %f direction: %f", vettrelplayers[i].distance, vettrelplayers[i].direction);
aball_data temp_ab = getAbsBall(&blackboard);
if (temp_ab.time != -1) {
	printf ("\n stampo dati assoluti palla:");
	printf ("\n x: %f y: %f direction: %f dir_change: %f dist_change: %f", temp_ab.x, temp_ab.y, temp_ab.direction, temp_ab.dir_change, temp_ab.dist_change);
	}
printf ("\n stampo giocatori visti assoluti:");
aplayer_data * vettabsplayers = (aplayer_data *) malloc (ALOTOF_PLAYERS * sizeof (aplayer_data));
getAbsPlayers (&blackboard, &num_players, vettabsplayers);
for (i=0; i<num_players; i++)
	if (vettabsplayers[i].time != -1)
		printf("\n x: %f y: %f direction: %f", vettabsplayers[i].x, vettabsplayers[i].y, vettabsplayers[i].direction);
}

void print_io () {

int now=getTime(&blackboard);
body_data io_a = getBody(&blackboard);
err_msg ("%i - x: %f y: %f head_angle: %f body_angle: %f",now, io_a.x, io_a.y, io_a.head_angle,io_a.body_angle);
}
*/

int sign(float a) {

if (a >= 0)
    return 1;
else
    return -1;
}


