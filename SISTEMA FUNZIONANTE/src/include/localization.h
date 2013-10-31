#ifndef LOCALIZATION2D_H_INCLUDED
#define LOCALIZATION2D_H_INCLUDED

#include "blackboard.h"
#include "errlib.h"

#define TOTALE_RIGHE 13
#define TOTALE_COLONNE 17
#define FIELD_LENGTH 115
#define FIELD_HEIGHT 78

#define DEG_RAD 0.017453293
#define RAD_DEG 57.295779506



typedef struct vett_indici_s{

int *vett;
int presenti;
int num_pali;

} vett_indici;

typedef struct objet_s {

    char *name;
    char *team;
    int id;
    float dist_x;
    float dist_y;
    float angle;

} objet;

typedef struct offset_s {

float offset_x;
float offset_y;

} offset;


vett_indici vett_base_righe[TOTALE_RIGHE];
vett_indici vett_base_colonne [TOTALE_COLONNE];
float vett_angoli_interni[3];
offset vett_offset [N_FLAGS];


void init();
void print();
void print_io();
int localization();
int sign (float);

#endif // LOCALIZATION2D_H_INCLUDED
