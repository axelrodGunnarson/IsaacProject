#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include "localization.h"
#include "blackboard.h"

#define FLAG_ASSENTE -1
#define RADIUS_BASE 1

int scelta;
body_data io;
int errore_pos;

void cerca_minima_distanza (int *, int *, int *);
void doppia_colonna (int, int, int);
void doppia_riga (int, int, int);
boolean calcolo_offset (int, int, int, int, int);
void calcola_angoli (int, int, int);
void calcola_angolazione();
int trova_io(int);
void trova_palla();
void trova_giocatori();
aplayer_data calcola_posizione_giocatori (rplayer_data);
int cerca_palo_vicino();
boolean escludi (int *, int, int);

/*
void sperimentale();
void calcolo_offset_sperimentale(int, int, int);
void cerca_minima_distanza_sperimentale(int *, int *);
void calcola_angoli_sperimentale (int, int);
*/


extern sem_t s_localization, s_localizationComplete, s_seeParsed;


int localization()
{
//int modalita;

clock_t start, end;
double nsec=0;
struct timespec ts;
int lock;


/*ts.tv_sec=0;
ts.tv_nsec=(SENSE_STEP_MS/3)*(1e6); // AKA 50ms with the standard step of 150ms for each vision msg*/

ts.tv_sec=0;
ts.tv_nsec=(0.3*SENSE_STEP_MS)*(1e6); // AKA 50ms with the standard step of 150ms for each vision msg




init(); //initizalization of offset vectors

//memset(&io,0,sizeof(io));

sleep (1);
while (1) {
    sem_wait(&s_localization);
    start = clock();

   	errore_pos=0;

	lock=sem_timedwait(&s_seeParsed, &ts); //wait see msg 50ms, if it timed out, apply offset! else do normal work
	if(lock==0){
		trova_io(1);
	} else
		errore_pos=1;

	if (errore_pos == 0 ){
		trova_palla();
		trova_giocatori();

		updateBody(&blackboard, io);
		resetOffset(&blackboard);
	}else{
		applyOffset(&blackboard);
		//LUK
		/*body_data body=getBody(&blackboard);
		io.x=body.x;
		io.y=body.y;
		io.body_angle=body.body_angle;
		io.head_angle=0;*/
		//trova_palla();
		//trova_giocatori();
    }

	//print();
	sem_post(&s_localizationComplete);
	end=clock();
	nsec=(((double)end-start)/CLOCKS_PER_SEC)*(1e9);
    if(nsec>SIMULATION_STEP_MS*(1e6))
        err_msg("localization() - DANGER, time is too fast!");
}

	return 0;
}

int trova_io (int modalita) {

//function who finds position f player

int palo_1, palo_2;
int scelta;

palo_1 = -1;

if (modalita == 1)
    {
    cerca_minima_distanza(&palo_1, &palo_2, &scelta);
    if (palo_1 != -1)
        {
        if (scelta == 0)
            doppia_colonna(palo_1, palo_2, scelta);
        else if (scelta == 1)
            doppia_riga(palo_1, palo_2, scelta);
        io.time=getTime(&blackboard);
        }
    else
        errore_pos = 1;
    }

/*
else if (modalita == 2)
    {
    cerca_minima_distanza_sperimentale(&palo_1, &palo_2);
    if (palo_1 != -1)
        sperimentale();
    else
        errore_pos=1;
    }
*/

io.x = io.x - FIELD_LENGTH/2;		//offset to move the reference system on the center of the field
io.y = - (io.y - FIELD_HEIGHT/2);


if (errore_pos == 0)
    return 0;

return 1;
}

void trova_palla () {

//function to find the ball

io.x = io.x + FIELD_LENGTH/2;		//move the offset to the left-low angle
io.y = + (- io.y + FIELD_HEIGHT/2);

float angolo, dir_change;

angolo=0;

rball_data ball_rel = getRelBall(&blackboard);
if (ball_rel.time == FLAG_ASSENTE)
    return;
aball_data ball_abs;

if (ball_rel.direction > 0)     //sum or sub of the angles dependant from where is the ball w.r.t. the player
    angolo = fabs (io.body_angle - io.head_angle) - fabs (ball_rel.direction);
else if (ball_rel.direction < 0)
    angolo = fabs (io.body_angle -io.head_angle) + fabs (ball_rel.direction);
else
    angolo =0;

if (angolo < 0)
    angolo=360+angolo;
if (angolo > 360)
    angolo=angolo-360;


ball_abs.x = cos(angolo*DEG_RAD)*ball_rel.distance + io.x;  //absolute coordinates
ball_abs.y = sin(angolo*DEG_RAD)*ball_rel.distance + io.y;
ball_abs.direction = angolo;

dir_change = ball_rel.dir_change + angolo;    //speed calculation
if (dir_change < 0)
    dir_change=360+dir_change;
if (dir_change > 360)
    dir_change=dir_change-360;
ball_abs.dir_change = dir_change;
ball_abs.dist_change = ball_rel.dist_change;

ball_abs.x = ball_abs.x - FIELD_LENGTH/2;   //offset
ball_abs.y = - (ball_abs.y - FIELD_HEIGHT/2 );

updateAbsBall(&blackboard, ball_abs);

io.x = io.x - FIELD_LENGTH/2;		//offset to move the reference system on the center of the field
io.y = - (io.y - FIELD_HEIGHT/2);
}

void trova_giocatori () {

//function to find the players on the field

rplayer_data *vett_players;
aplayer_data temp;
int num_players, i;
float dir_change;

vett_players = (rplayer_data *) malloc (ALOTOF_PLAYERS*sizeof(rplayer_data));

getRelPlayers(&blackboard, &num_players, vett_players);
for (i = 0; i < num_players; i++)
     {
     temp = calcola_posizione_giocatori (vett_players[i]);
     temp.x = temp.x - FIELD_LENGTH/2;  //offset
     temp.y = - (temp.y - FIELD_HEIGHT/2);

     dir_change = vett_players[i].dir_change + temp.direction; //speed
     	if (dir_change < 0)
     dir_change=360+dir_change;
	if (dir_change > 360)
     dir_change=dir_change-360;
     temp.dir_change = dir_change;
     temp.dist_change = vett_players[i].dist_change;
     temp.radius =  RADIUS_BASE* sqrt (pow ( (temp.x- io.x) / vett_players[i].distance, 2) + pow ( (temp.y - io.y) /vett_players[i].distance, 2) );

     updateAbsPlayer(&blackboard, temp);
     }
free(vett_players);
}

aplayer_data calcola_posizione_giocatori (rplayer_data giocatore) {

//calculate the position for a player

io.x = io.x + FIELD_LENGTH/2;		//move the offset to the left-low angle
io.y = + (- io.y + FIELD_HEIGHT/2);

aplayer_data temp;
float angolo;

if (giocatore.direction > 0)
    angolo = fabs (io.body_angle -io.head_angle) - fabs (giocatore.direction);
else if (giocatore.direction < 0)
    angolo = fabs (io.body_angle -io.head_angle) + fabs (giocatore.direction);
else
    angolo =0;

if (angolo < 0)
    angolo=360+angolo;
if (angolo > 360)
    angolo=angolo-360;
temp.x = cos(angolo*DEG_RAD)*giocatore.distance + io.x;
temp.y = sin(angolo*DEG_RAD)*giocatore.distance + io.y;

temp.direction = angolo;

temp.body_face_dir = angolo + giocatore.body_face_dir;
if (temp.body_face_dir < 0)
    temp.body_face_dir=360+temp.body_face_dir;
if (temp.body_face_dir > 360)
    temp.body_face_dir=temp.body_face_dir-360;

temp.head_face_dir = temp.body_face_dir - giocatore.head_face_dir;
if (temp.head_face_dir < 0)
    temp.head_face_dir=360+temp.head_face_dir;
if (temp.head_face_dir > 360)
    temp.head_face_dir=temp.head_face_dir-360;

io.x = io.x - FIELD_LENGTH/2;		//offset to move the reference system on the center of the field
io.y = - (io.y - FIELD_HEIGHT/2);

return temp;
}

void cerca_minima_distanza (int * palo_1, int * palo_2, int *scelta) {

/*calculate the minimal medium distance between two poles on the same row (column) and the player*/

int i, minimo, dist_media;
*scelta=2;
minimo=9999;
for (i=0; i< TOTALE_COLONNE; i++)
    {
        int contatore_palo1,contatore_palo2;
        for (contatore_palo1=0; contatore_palo1< vett_base_colonne[i].num_pali; contatore_palo1++)
            if ( getRelFlag(vett_base_colonne[i].vett[contatore_palo1], &blackboard).time != FLAG_ASSENTE) //found first pole of a column
                    for (contatore_palo2=contatore_palo1+1; contatore_palo2 <(vett_base_colonne[i].num_pali); contatore_palo2++) //look for another one on the same column
                        if (getRelFlag(vett_base_colonne[i].vett[contatore_palo2], &blackboard).time != FLAG_ASSENTE)
                        {
                            dist_media= (getRelFlag(vett_base_colonne[i].vett[contatore_palo2], &blackboard).distance +getRelFlag(vett_base_colonne[i].vett[contatore_palo1], &blackboard).distance)/2;
                            if (dist_media < minimo)
                            {
                                minimo=dist_media;
                                *palo_1 = vett_base_colonne[i].vett[contatore_palo1];
                                *palo_2 = vett_base_colonne[i].vett[contatore_palo2];
                                *scelta=0;
                            }
                        }

    }
for (i=0; i< TOTALE_RIGHE; i++)
    {
        int contatore_palo1,contatore_palo2;
        for (contatore_palo1=0; contatore_palo1< vett_base_righe[i].num_pali; contatore_palo1++)
            if (getRelFlag(vett_base_righe[i].vett[contatore_palo1],&blackboard).time != FLAG_ASSENTE)

                    for (contatore_palo2=contatore_palo1+1; contatore_palo2 <(vett_base_righe[i].num_pali); contatore_palo2++)
                        if (getRelFlag(vett_base_righe[i].vett[contatore_palo2], &blackboard).time != FLAG_ASSENTE)
                        {
                            dist_media= (getRelFlag(vett_base_righe[i].vett[contatore_palo2], &blackboard).distance +getRelFlag(vett_base_righe[i].vett[contatore_palo1], &blackboard).distance)/2;
                            if (dist_media < minimo)
                            {
                                minimo=dist_media;
                                *palo_1 = vett_base_righe[i].vett[contatore_palo1];
                                *palo_2 = vett_base_righe[i].vett[contatore_palo2];
                                *scelta=1;
                            }
                        }

    }
}

void doppia_riga (int palo_1, int palo_2, int scelta) {

/*position of the player if the two poles belong to the same row
3 possible cases:
1. both the angles on the poles are less than 90° (player between the poles)
2. the angle for palo_1 is > 90° (player above the poles)
3. the angle for palo_2 is > 90° (player below the poles)
*/

calcola_angoli(palo_1, palo_2, scelta);

int pivot, maggiore, minore, caso;
float dist_x, dist_y;
io = getBody(&blackboard);

if (vett_angoli_interni[0]<90 && vett_angoli_interni[1]<90) //caso 1
    {
    caso=1;
    if (vett_angoli_interni[0]<vett_angoli_interni[1])
        {
        minore=0;
        pivot=palo_1;
        }
    else
        {
        minore=1;
        pivot=palo_2;
        }
    dist_x = getRelFlag(pivot, &blackboard).distance*cos(vett_angoli_interni[minore]*DEG_RAD);
    dist_y = getRelFlag(pivot, &blackboard).distance*sin(vett_angoli_interni[minore]*DEG_RAD);

    io.x = dist_x; //relative
    io.y = dist_y;

    boolean sotto = calcolo_offset(palo_1, caso, palo_1, palo_2, scelta);

    if (sotto)
        dist_y = vett_offset[palo_1].offset_y - dist_y;
    else
        dist_y = vett_offset[palo_1].offset_y + dist_y;
    dist_x = vett_offset[palo_1].offset_x + dist_x; //palo_1 will always be the one on the left
    }
else
    {
    if (vett_angoli_interni[0]>90)
        {
        maggiore = 0;
        minore = 1;
        pivot = palo_2;
        caso=2;
        }
    else
        {
        maggiore = 1;
        minore = 0;
        pivot = palo_1;
        caso=3;
        }

    dist_y = getRelFlag(pivot, &blackboard).distance*sin(vett_angoli_interni[minore]*DEG_RAD);
    dist_x= sqrt ( pow(getRelFlag(pivot, &blackboard).distance, 2) - pow(dist_y, 2));

    io.x = dist_x;
    io.y = dist_y;

    boolean sotto = calcolo_offset(pivot, caso, palo_1, palo_2, scelta);

    vett_angoli_interni[maggiore]=180-vett_angoli_interni[maggiore];
    if (sotto)
        dist_y = vett_offset[palo_1].offset_y -dist_y;
    else
        dist_y = vett_offset[palo_1].offset_y + dist_y;

    if (pivot == palo_1)
        dist_x = vett_offset[pivot].offset_x + dist_x;
    else
        dist_x = vett_offset[pivot].offset_x - dist_x;
    }

io.x = dist_x;
io.y = dist_y;
calcola_angolazione(); //calculate the angulation of the player w.r.t. the horizontal positive semiaxis

io.direction_speed = io.direction_speed + io.body_angle; //direction of the speed

if (io.direction_speed < 0)
    io.direction_speed=360+io.direction_speed;
if (io.direction_speed > 360)
    io.direction_speed=io.direction_speed-360;

return;
}

void doppia_colonna(int palo_1, int palo_2, int scelta) {

//same function as doppia_riga

calcola_angoli(palo_1, palo_2, scelta);

int dist_x, dist_y, pivot, maggiore, minore;
int caso;
io = getBody(&blackboard);

if (vett_angoli_interni[0]<90 && vett_angoli_interni[1]<90)
    {
    caso=1;
    minore=0;
    pivot=palo_1;

    dist_x = getRelFlag(pivot, &blackboard).distance*sin(vett_angoli_interni[minore]*DEG_RAD);
    dist_y = getRelFlag(pivot, &blackboard).distance*cos(vett_angoli_interni[minore]*DEG_RAD);

    io.x = dist_x;
    io.y = dist_y;

    boolean destra = calcolo_offset(pivot, caso, palo_1, palo_2, scelta);
    if (destra == TRUE)
        dist_x = vett_offset[pivot].offset_x + dist_x;
    else
        dist_x = vett_offset[pivot].offset_x - dist_x;
    if (pivot == palo_1)
        dist_y = vett_offset[pivot].offset_y - dist_y;//palo_1 will always be the one above
    else
        dist_y = vett_offset[pivot].offset_y + dist_y;
    }
else
    {
    if (vett_angoli_interni[0]>90)
        {
        maggiore = 0;
        minore = 1;
        pivot = palo_2;
        caso = 2;
        }
    else
        {
        maggiore = 1;
        minore = 0;
        pivot = palo_1;
        caso=3;
        }
    vett_angoli_interni[maggiore]=180-vett_angoli_interni[maggiore];
    dist_x = getRelFlag(pivot, &blackboard).distance*sin(vett_angoli_interni[minore]*DEG_RAD);
    dist_y= sqrt ( pow(getRelFlag(pivot, &blackboard).distance, 2) - pow(dist_x, 2));
    io.x = dist_x;
    io.y = dist_y;

    boolean destra = calcolo_offset(pivot, caso, palo_1, palo_2, scelta);

    if (destra)
        dist_x = vett_offset[palo_1].offset_x + dist_x;
    else
        dist_x = vett_offset[palo_1].offset_x - dist_x;

    if (pivot == palo_1)
        dist_y = vett_offset[palo_1].offset_y - dist_y;
    else
        dist_y = vett_offset[palo_2].offset_y + dist_y;
    }

io.x = dist_x;
io.y = dist_y;
calcola_angolazione();

io.direction_speed = io.direction_speed + io.body_angle; //direction of the speed

if (io.direction_speed < 0)
    io.direction_speed=360+io.direction_speed;
if (io.direction_speed > 360)
    io.direction_speed=io.direction_speed-360;
return;
}

boolean calcolo_offset (int pivot, int caso, int palo_1, int palo_2, int scelta) {

int vicino;
float dist_y_stima, dist_x_stima;
float distanza_x_1, distanza_x_2;
float distanza_y_1, distanza_y_2;
float distanza_x, distanza_y;
float distanza_1, distanza_2;
float err_1, err_2;

/*function to understand if the player is above or below (left or right) respect to the considered row (column)
it hypotize one of the two and then compare the distance in data with the distance otained by calculation respect to another pole*/

//column and row cases are separated

if (scelta == 0)
    {
    if (caso ==2)
            dist_y_stima = vett_offset[pivot].offset_y + io.y;
    else
            dist_y_stima = vett_offset[pivot].offset_y - io.y;

    dist_x_stima = io.x + vett_offset[pivot].offset_x;	//supposed player to the right respect to the pole

    int vett_esclusi[2] = {palo_1, palo_2};

    vicino = cerca_palo_vicino(vett_esclusi,2);
    if (vicino == -1)
        {
        errore_pos=1;
        return 1;
        }
    distanza_x_1 = dist_x_stima - vett_offset [vicino].offset_x; //distance if the playre is at the right of the column
    distanza_y = dist_y_stima - vett_offset[vicino].offset_y;
    distanza_1 = sqrt( pow(distanza_x_1, 2) + pow (distanza_y, 2) );

    distanza_x_2 = dist_x_stima - 2*io.x - vett_offset[vicino].offset_x; //left
    distanza_2 = sqrt( pow(distanza_x_2, 2) + pow (distanza_y, 2) );

    err_1 = fabs (distanza_1 - getRelFlag(vicino, &blackboard).distance);    //errors
    err_2 = fabs (distanza_2 - getRelFlag(vicino, &blackboard).distance);

    if (err_1 <= err_2)
        return TRUE;
    else
        return FALSE;

    }
if (scelta == 1)
    {
    dist_y_stima = vett_offset[pivot].offset_y - io.y;	//supposed player below the pole

    if (caso == 2)
        dist_x_stima = vett_offset[pivot].offset_x - io.x;
    else
        dist_x_stima = vett_offset[pivot].offset_x + io.x;

    int vett_esclusi[2] = {palo_1, palo_2};

    vicino = cerca_palo_vicino(vett_esclusi,2);
    if (vicino == -1)
        {
        errore_pos = 1;
        return FALSE;
        }
    distanza_x = dist_x_stima - vett_offset[vicino].offset_x; //below the row
    distanza_y_1 = dist_y_stima - vett_offset[vicino].offset_y;
    distanza_1 = sqrt( pow(distanza_x, 2) + pow (distanza_y_1, 2) );

    distanza_y_2 = dist_y_stima + 2*io.y - vett_offset[vicino].offset_y; //above
    distanza_2 = sqrt( pow(distanza_x, 2) + pow (distanza_y_2, 2) );

    err_1 = fabs (distanza_1 - getRelFlag(vicino, &blackboard).distance);
    err_2 = fabs (distanza_2 - getRelFlag(vicino, &blackboard).distance);

    if (err_1 <= err_2)
        return TRUE;
    else
        return FALSE;
    }
	return 0;

}

void calcola_angoli(int palo_1, int palo_2, int scelta) {

float bordo;
double param;

//function to calculate the internal angles of the triangle palo_1,palo_2,player

//vett_angoli interni: angolo_palo_1 | angolo_palo_2 | angolo_player

if ( (getRelFlag(palo_1, &blackboard).direction >=0 && getRelFlag(palo_2, &blackboard).direction>=0) || (getRelFlag(palo_1, &blackboard).direction<=0 && getRelFlag(palo_2, &blackboard).direction<=0) )
    vett_angoli_interni[2] = fabs ( getRelFlag(palo_1, &blackboard).direction - getRelFlag(palo_2, &blackboard).direction );
else
    vett_angoli_interni[2] = fabs (getRelFlag(palo_1, &blackboard).direction) + fabs (getRelFlag(palo_2, &blackboard).direction );

if (scelta ==0)
    bordo = fabs(vett_offset[palo_1].offset_y - vett_offset[palo_2].offset_y);
else
    bordo = fabs (vett_offset[palo_1].offset_x - vett_offset[palo_2].offset_x);

param = (pow(getRelFlag(palo_1, &blackboard).distance,2) + pow(bordo,2) - pow(getRelFlag(palo_2, &blackboard).distance,2)) / (2*getRelFlag(palo_1, &blackboard).distance*bordo);
if (param > 1)
    param=1;
if (param < -1)
    param = -1;
vett_angoli_interni[0] = acos ( param )*RAD_DEG;

param = (pow(getRelFlag(palo_2, &blackboard).distance,2) + pow(bordo,2) - pow(getRelFlag(palo_1, &blackboard).distance,2)) / (2*getRelFlag(palo_2, &blackboard).distance*bordo);
if (param > 1)
    param=1;
if (param < -1)
    param = -1;

vett_angoli_interni[1] = acos ( param )*RAD_DEG;


}

void calcola_angolazione () {

/*function to calculate the direction of the player: it takes one pole and apply the offset to the relative orientation depending on the position of the pole*/


float angolo, alfa, alfa1;
float dist_x, dist_y;
int quadrante;

int pivot = cerca_palo_vicino(NULL, 0); //find nearest pole

if (pivot == -1)
    {
    io.body_angle=0;
    errore_pos=1;
    return;
    }
/*quadrant where the pole is*/

dist_x = vett_offset[pivot].offset_x - io.x;
dist_y = vett_offset[pivot].offset_y - io.y;

if (dist_x >= 0)
    {
    if (dist_y >=0)
        quadrante = 1;
    else
        quadrante = 4;
    }
else
    {
    if (dist_y >= 0)
        quadrante = 2;
    else
        quadrante = 3;
    }

if (dist_y == 0)
    alfa1 = 90;
else
    {
    alfa1 = atan(fabs(dist_y) / fabs(dist_x)) * RAD_DEG;
    int diviso = alfa1 / 90;
    alfa = alfa1 - (90*diviso);
    }

if (quadrante%2 == 0) //quadrant 2 or 4
    {
    angolo = alfa - getRelFlag(pivot, &blackboard).direction - io.head_angle;
    if (quadrante == 2)
        io.body_angle = 180 - angolo;
    else
        io.body_angle = 360 - angolo;
    }
else    //1 or 3
    {
    angolo = alfa + getRelFlag(pivot, &blackboard).direction - io.head_angle;
    if (quadrante == 1)
        io.body_angle = angolo;
    else
        io.body_angle = 180 + angolo;
    }

io.body_angle=-io.body_angle;

while(io.body_angle>360)
	io.body_angle-=360;
while(io.body_angle<-360)
	io.body_angle+=360;
if(io.body_angle>180)
	io.body_angle=-(360-io.body_angle);
if(io.body_angle<-180)
	io.body_angle=360+io.body_angle;

}

int cerca_palo_vicino (int *vettore_esclusi, int dim_vett) {

int minimo, i;
float minima_dist = 9999;
minimo=-1;

//function to calculate the nearest pole

for (i=0; i< N_FLAGS; i++)
    if (getRelFlag(i, &blackboard).distance < minima_dist && getRelFlag(i, &blackboard).time != FLAG_ASSENTE && escludi (vettore_esclusi,dim_vett,i) == FALSE)
        {
         minimo = i;
         minima_dist = getRelFlag(minimo, &blackboard).distance;
        }
return minimo;

}

boolean escludi (int *vett_esclusi, int dim_vett, int palo) {

//function to exlude some poles from the research of the minimum

if (dim_vett == 0 || vett_esclusi == NULL)
    return FALSE;

int j;
for (j=0; j < dim_vett; j++)
    if (vett_esclusi[j] == palo)
        return TRUE;
return FALSE;
}



/*
void cerca_minima_distanza_sperimentale (int *palo_1, int *palo_2) {

*palo_1 = cerca_palo_vicino(NULL, 0);
int vett_esclusi[1];
vett_esclusi [0] = *palo_1;
*palo_2 = cerca_palo_vicino (vett_esclusi, 1);
}


void sperimentale () {

int palo_1, palo_2;
float alfa, angolo;

cerca_minima_distanza_sperimentale(&palo_1, &palo_2);

calcola_angoli_sperimentale(palo_1, palo_2);

//printf ("\nangoli interni: palo_1 : %f, palo_2: %f, giocatore: %f", vett_angoli_interni[0], vett_angoli_interni[1], vett_angoli_interni[2]);

int pivot= palo_1;
int n_pivot = 0;

float os_x = fabs ( vett_offset[palo_1].offset_x - vett_offset [palo_2].offset_x);
float os_y = fabs ( vett_offset[palo_1].offset_y - vett_offset [palo_2].offset_y);


alfa = atan(os_y/os_x)*RAD_DEG;

if (vett_angoli_interni[2] + alfa < 90)
    angolo = 90 - alfa - vett_angoli_interni[2];
else
    angolo = 180 - alfa -vett_angoli_interni[2];

//TODO: risolvere il problema su quale sia il palo sul quale è incentrata la misura di alfa


io.x = getRelFlag(pivot, &blackboard).distance*sin(angolo);
io.y = getRelFlag(pivot, &blackboard).distance*cos(angolo);

//printf ("\n palo_1 = %d, palo_2 = %d, pivot = %d, angolo risultante = %f \n gli offset sono x: %f y: %f \n misure relative x: %f, y: %f", palo_1, palo_2, pivot, angolo, os_x, os_y, io.x, io.y);

calcolo_offset_sperimentale (pivot, palo_1, palo_2);

calcola_angolazione();
calcola_velocita_io();

}

void calcolo_offset_sperimentale (int pivot, int palo_1, int palo_2) {

int vett_esclusi[2] = {palo_1, palo_2};

int palo = cerca_palo_vicino (vett_esclusi, 2);

if (palo == -1)
    {
    errore_pos=1;
    return;
    }

float vett_err[4];
float temp_x = vett_offset[pivot].offset_x - vett_offset[palo].offset_x;
float temp_y = vett_offset[pivot].offset_y - vett_offset[palo].offset_y;
vett_err[0] = sqrt( pow(temp_x + io.x, 2) + pow (temp_y + io.y, 2 ) ) - getRelFlag(palo, &blackboard).distance;
vett_err[1] = sqrt( pow(temp_x - io.x, 2) + pow (temp_y + io.y, 2 ) ) - getRelFlag(palo, &blackboard).distance;
vett_err[2] = sqrt( pow(temp_x - io.x, 2) + pow (temp_y - io.y, 2 ) )- getRelFlag(palo, &blackboard).distance;
vett_err[3] = sqrt( pow(temp_x + io.x, 2) + pow (temp_y - io.y, 2 ) ) - getRelFlag(palo, &blackboard).distance;

int num = 0;
int i;
for (i=1; i<4; i++)
	if (vett_err[i] < vett_err[num])
		num = i;
switch (num) {
	case 0:
        io.x = vett_offset[pivot].offset_x + io.x;
		io.y = vett_offset[pivot].offset_y + io.y;
		break;
	case 1:
        io.x = vett_offset[pivot].offset_x - io.x;
		io.y = vett_offset[pivot].offset_y + io.y;
		break;
	case 2:
        io.x = vett_offset[pivot].offset_x - io.x;
		io.y = vett_offset[pivot].offset_y - io.y;
		break;
	case 3:
        io.x = vett_offset[pivot].offset_x + io.x;
		io.y = vett_offset[pivot].offset_y + io.y;
		break;
	}
//printf ("\n palo di offset: %d, nel quadrante %d+1", palo, num );
}


void calcola_angoli_sperimentale (int palo_1, int palo_2) {

//calcolo dell'angolo riferito al player, salvato in vett_ang[2]
if ( (getRelFlag(palo_1, &blackboard).direction >=0 && getRelFlag(palo_2, &blackboard).direction>=0) || (getRelFlag(palo_1, &blackboard).direction<=0 && getRelFlag(palo_2, &blackboard).direction<=0) )
    vett_angoli_interni[2] = fabs ( getRelFlag(palo_1, &blackboard).direction - getRelFlag(palo_2, &blackboard).direction );
else
    vett_angoli_interni[2] = fabs (getRelFlag(palo_1, &blackboard).direction) + fabs (getRelFlag(palo_2, &blackboard).direction );

float bordo = sqrt ( pow(getRelFlag(palo_1, &blackboard).distance,2) + pow(getRelFlag(palo_2, &blackboard).distance,2) - 2*getRelFlag(palo_1, &blackboard).distance*getRelFlag(palo_2, &blackboard).distance*cos(vett_angoli_interni[2]));

double param = (pow(getRelFlag(palo_1, &blackboard).distance,2) + pow(bordo,2) - pow(getRelFlag(palo_2, &blackboard).distance,2)) / (2*getRelFlag(palo_1, &blackboard).distance*bordo);
if (param > 1)
    param=1;
if (param < -1)
    param = -1;
vett_angoli_interni[0] = acos ( param )*RAD_DEG;

param = (pow(getRelFlag(palo_2, &blackboard).distance,2) + pow(bordo,2) - pow(getRelFlag(palo_1, &blackboard).distance,2)) / (2*getRelFlag(palo_2, &blackboard).distance*bordo);
if (param > 1)
    param=1;
if (param < -1)
    param = -1;

vett_angoli_interni[1] = acos ( param )*RAD_DEG;


}
*/

