#ifndef _MENU_H
#define _MENU_H

/*module permettant la gestion du menu de choix de l'utilisateur
 * au d�but, le menu est peu �volu�*/

typedef enum
{
  MENU_INTRODUCTION,
  MENU_GAME_OVER_PERD,
  MENU_GAME_OVER_GAGNE,
  MENU_NIVEAU_REUSSI,
  MENU_NIVEAU_PERDU
} Menu_etat;

/* pour savoir ou on en est dans le menu*/

/*pour savoir ce que veut faire le joueur*/
typedef enum
{
  MENU_QUITTER,
  MENU_CONTINU,
  MENU_UN_JOUEUR,
  MENU_UN_JOUEUR_MODE_FLO
        
} Menu_resultat;

  
/* on attend de savoir ce que veut faire le joueur en partant d'un �tat etat*/
Menu_resultat menu(G_fenetre* fenetre, Menu_etat etat);

#endif
