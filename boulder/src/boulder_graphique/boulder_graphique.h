#ifndef _BOULDER_GRAPHIQUE_H
#define _BOULDER_GRAPHIQUE_H


/* Ce module propose des fonctions graphiques avanc�s pour le jeu
 Comme par exemple, la gestion des sprites au format du jeu, l'affichage 
 de ceux-ci (en anim�)
 */



/*Structure contenant tous les sprites possibles (avec toutes les animations possibles
 *suivant le niveau*/
typedef struct
{
  G_surface* joueur_bouge_pas[1];
  G_surface* joueur_gauche[4];
/*  G_surface* joueur_bas[4];*/
  G_surface* joueur_droite[4];
/*  G_surface** joueur_haut;*/
  G_surface* mur[1];
  G_surface* mur_qui_bouge[4];
  G_surface* pierre[1];
  G_surface* diamant[4];
  G_surface* ennemi_type_rien[4];
  G_surface* ennemi_type_diamant[4];
  G_surface* slim[2];
  G_surface* vide[1];
  G_surface* terre[1];
  G_surface* bord[2];   
  G_surface* explosion[3];
  G_surface* diamant_type_transformation[4];
  G_surface* bombe[1];
  int compteur_diamant;   
  int compteur_mur_qui_bouge;   
  int compteur_joueur;
  int compteur_ennemi_type_rien;  
  int compteur_ennemi_type_diamant;
  int compteur_slim;
  int compteur_porte_fin;
  
} Ecran_joueur_sprite;


/* Structure contenant toutes les couleurs du niveau courant
 La structure change � chaque niveau*/
typedef struct {
   G_couleur couleur_noir;
   G_couleur couleur_blanc;
   G_couleur couleur_tete;
   G_couleur couleur_jambe;
   G_couleur couleur_moche;
} Ecran_joueur_couleurs;        




/*
 * Structure contenant toutes les infomations pour le joueur courant
 *  Le nombre de sprite en largeur et en longueur, le pointeur vers l'ensemble
 * des sprites, la r�solution, le facteur de zoom, la taille des sprites,
 * le niveau en cours, la position actuel dans le niveau,
 * si le niveau doit �tre repositionn� (par exemple si le joueur va trop � gauche,
 * le niveau doit �tre d�cal�), les indices de d�calages (le niveau doit 
 * par exemple �tre d�clag� de 5 sprites si le joueur va trop � gauche,
 * il faut donc un indice pour conna�tre ce nombre)
 *
 * Et finalement, un pointeur vers le joueur sur lequel s'applique cette structure
 */
typedef struct
{
   Ecran_joueur_sprite* ensemble_sprite;
   int nombre_de_spriteX;
   int nombre_de_spriteY;
   int resolutionx;
   int resolutiony;
   int zoom;
   int taille_spritex;
   int taille_spritey;
   
   Niveau* niveau;
   Joueur* j_courant;
   
   
   enum{ REPOSITIONNEMENT_GAUCHE,
            REPOSITIONNEMENT_DROITE,
            REPOSITIONNEMENT_HAUT,
            REPOSITIONNEMENT_BAS,
            REPOSITIONNEMENT_BOUGE_PAS
            } repositionnement;
   int j_boucle_decalage;
   int j_decalagex;
   int j_decalagey;
   int j_commence_decalagex;
   int j_commence_decalagey;
   float j_originex;
   float j_originey;
   
   
   Ecran_joueur_couleurs* couleurs;
   
   G_surface* surface;
   G_rectangle position_texte;
   G_font* font;
} Ecran_joueur;



/**********
 Ecran_joueur
 ************/

/*
 Fonction permettant � partir d'un �cran joueur, de la position que l'on souhaite
   convertir et de l'�l�ment � afficher, renvoie le sprite G_surface � afficher
   Cela fait attention au compteur graphique pour les animations, c'est pour
   cela que l'�cran_joueur est n�cessaire. La position est n�cessaire car certains
   �l�ments ont diff�rents types.
 */
G_surface* element_niveau2sprite(Ecran_joueur* ecran, Niveau_element element, int i, int j);


void                   ecran_joueur_spriteEvoluer (Ecran_joueur_sprite* ensemble);


/*
 * Constructeur de structure, initialise en m�me tps les champs
 * x correspond � la r�solution horizontal pour le joueur et y pour la vertical
 */
Ecran_joueur*          ecran_joueurCreer(int x, int y);

/*
 * Initialise un �cran joueur suivant un niveau, un joueur, le nombre de sprite
  horizontal et vertical, le facteur de zoom et le font
 */
void                   ecran_joueurInitialiser( Ecran_joueur* ecran, Niveau* niveau, 
                                                     Joueur* joueur,
                                                     int nombre_de_spriteX,
                                                     int nombre_de_spriteY,
                                                     int zoom,
                                                     G_font* font);
                                             
/*
 * Permet de r�aliser les calculs pour repositionner les origines du niveau
 * (pour pas que le joueur sorte du cadre mais que le niveau d�file
 */ 
void                   ecran_joueurRepositionnementOrigine(Ecran_joueur* ecran);

/*
 * Repositionne les origines en 1, 1
 */
void                   ecran_joueurCalculOrigine(Ecran_joueur* ecran);


/*
 * Permet l'affichage de tout le niveau pour le joueur � l'�cran
 */
void                   ecran_joueurRaffraichissement(Ecran_joueur*);


/*
 * Permet de charger les couleurs du niveau
 */
void                   ecran_joueurChargementCouleurs(Ecran_joueur* ecran, char* fichier);


/*
 * Ouvre un fichier de sprite et retourne un pointeur graphique vers celui-ci
 * La fonction n�cessite un �cran_joueur car le sprite d�pend de la couleur du niveau
 */
G_surface*             ecran_joueurChargementSprite(Ecran_joueur* ecran,  char* fichier);

/*
 * Fonction accesseur, retourne toute la partie graphique de l'�cran
 */
G_surface*             ecran_joueurLireSurface(Ecran_joueur* ecran);

/*
 * Permet de d�truire l'ensemble des sprites allou�s
 */
void                   ecran_joueurDetruireSprite(Ecran_joueur* ecran);


/*
 * Permet de d�sallou�e toute la m�moire allou�e � l'�cran
 */
void                   ecran_joueurDetruire(Ecran_joueur* ecran);

/*
 * Accesseur : Retourne le niveau de l'�cran joueur
 */
Niveau*                ecran_joueurLireNiveau(Ecran_joueur* ecran);


/*
 * Constructeur : Ecrit un niveau sur l'�cran joueur
 */
void                   ecran_joueurEcrireNiveau(Ecran_joueur* ecran, Niveau* niveau);

/*
 * Permet de faire �voluer les compteurs pour l'animation des sprites
 */
void ecran_joueurEvoluerCompteur(Ecran_joueur* ecran);


/*
 Accesseur : Renvoie la r�solution horizontale
 */
int ecran_joueurLireResolutionx (Ecran_joueur* ecran);

/*
 Accesseur : Renvoie la r�solution verticale
 */
int ecran_joueurLireResolutiony (Ecran_joueur* ecran);


/*
   Pour l'int�gration
   */
void ecran_joueurCopieFenetre(G_fenetre* fenetre, G_rectangle rect1, 
                                           Ecran_joueur* ecran, G_rectangle rect2);


/*
 * Ensembles de fonctions permettant le chargement de tous les sprites
 * Il y a par exemple 4 char* car il y a 4 sprites pour l'animation d'un personnage
 */
void ecran_joueurChargementSpriteJoueurBougePas (Ecran_joueur* ecran, char* s);
void ecran_joueurChargementSpriteJoueurGauche (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3, char* s4);
void ecran_joueurChargementSpriteJoueurDroite (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3, char* s4);
void ecran_joueurChargementSpriteMur (Ecran_joueur* ecran, char* s);
void ecran_joueurChargementSpriteMurQuiBouge (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3, char* s4);
void ecran_joueurChargementSpritePierre (Ecran_joueur* ecran, char* s);
void ecran_joueurChargementSpritePierre_type_explosion  (Ecran_joueur* ecran, char* s);

void ecran_joueurChargementSpriteDiamant (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3, char* s4);
void ecran_joueurChargementSpriteEnnemiTypeRien (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3, char* s4);

void ecran_joueurChargementSpriteEnnemiTypeDiamant (Ecran_joueur* ecran, 
                                              char* s1, char* s2); /*, char* s3, char* s4);*/
void ecran_joueurChargementSpriteVide (Ecran_joueur* ecran, char* s);
void ecran_joueurChargementSpriteTerre (Ecran_joueur* ecran, char* s);
void ecran_joueurChargementSpriteBord (Ecran_joueur* ecran, char* s, char* s2);
void ecran_joueurChargementSpriteSlim (Ecran_joueur* ecran, 
                                              char* s1, char* s2);
void ecran_joueurChargementSpriteExplosion (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3) /*, char* s4)*/;
                                              
void ecran_joueurChargementSpriteDiamantTypeTransformation (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3, char* s4);
                                              
#endif
