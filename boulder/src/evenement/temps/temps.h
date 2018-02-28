#include <SDL/SDL.h>

/* Module du temps (peu �volu�), utilise la SDL.
 Permet d'�tre ind�pendant de SDL*/

#define SECONDES *1000

typedef  float Temps;


/* permet de r�cuperer le temps actuel*/

Temps tempsRecuperer();

/* bool�en pour savoir si un temps a est inf�rieur � un temps b*/
short tempsInferieur(Temps a, Temps b);

/* additionne deux temps*/
Temps tempsSomme(Temps a, Temps b);

/* attend un certain temps*/
void  tempsAttendre(Temps a);
