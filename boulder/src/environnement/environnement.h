#ifndef _ENVIRONNEMENT_H
#define _ENVIRONNEMENT_H


/*Permet de faire �voluer chaque objet du niveau,
  par exemple, la chute des pierres, les ennemis qui bougent...
  */
void environnementEvoluer(Niveau* niveau);

/* Fait �voluer le slim*/
 void environnementEvoluerSlim (Niveau* niveau);
 

/*fonction publique d'�volution des explosions*/
void environnementEvoluerListeExplosion(Niveau* niveau);
void environnementCreerExplosion33(Niveau* niveau, Explosion_type type, int i, int j);


/*fonction publique d'�volution pour pierre*/
void environnementEvoluerListePierre(Niveau* niveau);


void environnementEvoluerListeJoueur(Niveau* niveau);
void environnementEvoluerListeDiamant(Niveau* niveau);
void environnementEvoluerListeEnnemi(Niveau* niveau);
void environnementEvoluerListeSlim(Niveau* niveau);



/*fonction priv�e d'�volution pour pierre*/
void environnementEvoluerPierre (Niveau* niveau, Pierre* pierre);
void environnementListePierreVidage( Niveau* niveau);

/*fonction priv�e d'explosion*/
/*utile pour les tests*/
void environnementCreerExplosion11(Niveau* niveau, Explosion_type type, int i, int j);
void environnementAjouterExplosion(Niveau* niveau, Explosion_type type, int i, int j);

/*prototypes pour joueur*/
void environnementMortJoueur(Niveau* niveau,Joueur * joueur);

#endif
