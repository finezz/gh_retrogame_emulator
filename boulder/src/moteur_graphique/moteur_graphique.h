#ifndef _MOTEUR_GRAPHIQUE_H
#define _MOTEUR_GRAPHIQUE_H
#include <SDL/SDL.h>

#ifndef SANS_TTF
#include <SDL/SDL_ttf.h>
#endif

/**************************
 * IMPORTANT :
 *
 * La cr�ation d'un module moteur_graphique, permet de rendre assez 
 * ind�pendant le jeu du moteur SDL
 */
 


/*Title: Module moteur_graphique
 * 
 */
 
 
/*Topic: Documentation du module moteur_graphique
 * Ce module permet la r�alisation d'application graphique en mode double buffering.
 * Il est compatible sous tous les syst�mes Windows, Linux, UNIX, Mac
 */
 

typedef SDL_Surface G_surface;
typedef SDL_Event G_evenement;


typedef SDL_Rect G_rectangle;


typedef Uint32 G_couleur;

#ifndef SANS_TTF


typedef TTF_Font G_font;
#else
typedef int G_font;
#endif

typedef enum {
        PLEIN_ECRAN,
        PAS_PLEIN_ECRAN
        } G_fenetre_mode;
        


typedef struct {
        int resolutionx;
        int resolutiony;
        G_surface* surface;
        G_fenetre_mode mode;
} G_fenetre;

/**Classes: couleur
 * Type abstrait de donn�e couleur utilis� dans les surfaces
 */
/*
 * Function: g_couleurCreer
 * Permet la cr�ation d'une couleur RGB en fonction d'une surface, il y a dependances � cause notamment
 * de diff�rence entre une surface 32 ou 16 bits par exemple.
 *
 * Param�tres:
 * surface - la couleur est d�finie selon la surface
 * R - niveau de rouge (entre 0 et 255)
 * G - niveau de vert (entre 0 et 255)
 * B - niveau de bleu (entre 0 et 255)
 *
 * Retour:
 * La couleur de niveau RGB
 *
 * Pr�condition:
 * R, G et B doivent �tre entre 0 et 255
 */
G_couleur g_couleurCreer(G_surface* surface, int R, int G, int B);

/**Classes: rectangle
 * Les copies de surface se r�alise � partir de ce type abstrait
 */
 
/*
 * Function: g_rectangleInitialiser
 * Permet d'initialiser un rectangle � une certaine position et d'une certaine taille
 *
 * Param�tres:
 * x - position horizontale
 * y - position verticale
 * w - taille horizontale
 * h - taille verticale
 *
 */
void g_rectangleInitialiser(G_rectangle* rect, int x, int y, int w, int h);



/*Classes: font
 * Le type abstrait font
*/
 /** Function: g_fontCharger
 * Permet de charger une police TTF � une certaine taille
 *
 * Param�tres:
 * fichier - le fichier du disque dur au format TTF
 * taille - la taille de la police voulue
 *
 * Retour: 
 *  Renvoit un pointeur de type G_font correpondant au font charg�
 *
 * Erreur:
 *  Si le fichier n'est pas trouv�, NULL est retourn�
 */
G_font*   g_fontCharger(char* fichier, int taille);
/** Function: g_fontDetruire
 * Permet la d�struction d'une font
 *
 * Param�tre:
 * font - la font que l'on souhaite d�truire
 */
void g_fontDetruire(G_font* font);


/*Classes: G_surface
 * Ce type abstrait est une surface graphique se trouvant en m�moire vid�o. Il est similaire � une image
 */

/* Function: g_surfaceCreer
 * Permet la cr�ation d'une surface d'une certaine taille, la surface est en 32 bits.
 *
 * Param�tres:
 * x - taille horizontale de la surface souhait�e
 * y - taille verticale de la surface souhait�e
 *
 * Retour:
 *  Retourne un pointeur vers la surface qui a �t� allou�e
 *
 * Erreur:
 *  Si la fonction n'a pas r�ussi � allouer la m�moire pour la surface, NULL est renvoy�
 */
G_surface* g_surfaceCreer(int x, int y);

/**Function: g_surfaceEcrireCouleur
 * Permet d'�crire � une certaine position d'une surface une couleur
 *
 * Param�tres:
 * surface - la surface ou l'on souhaite �crire
 * x - la position horizontale
 * y - la position verticale
 * couleur - la couleur que l'on souhaite �crire
*/
void       g_surfaceEcrireCouleur(G_surface* surface, int x, int y, G_couleur couleur);

/**Function: g_surfaceCopie
 * Permet de copier une surface vers une autre � certaines positions et � certaine taille
 *
 * Param�tres:
 * surface1 - la surface source
 * rect1 - le rectangle contient les origines et les longueurs que l'on souhaite copier
 * surface2 - la surface destination
 * rect2 - le rectangle contient les origines et les longueurs de l'endroit que l'on souhaite copier
 *
 * Si l'on souhaite copier toute la surface source, on peut pla�er rect1 � NULL, si l'on souhaite uniquement
 * plac� la source dans la destination � une certaine position sans se soucier des tailles, il suffit
 * d'initialiser un rectangle avec les tailles � 0
 */
void       g_surfaceCopie(G_surface* surface1, G_rectangle* rect1, G_surface* surface2, G_rectangle* rect2);

/**Function: g_surfaceEcrireTexte
 * Ecrit un texte dans une surface et renvoit la surface correspondante, la surface est allou�e automatiquement
 *
 * Param�tres:
 * font - le font de police que l'on souhaite
 * texte - le texte que l'on souhaite �crire
 * couleur_texte - la couleur du texte
 * couleur_font - la couleur du fond
 *
 * Retour:
 *  Retourne une surface contenant le texte aux couleurs indiqu�s et au font pr�cis�
 */
G_surface*      g_surfaceEcrireTexte(G_font* font, char* texte, G_couleur couleur_texte, G_couleur couleur_fond);

/**Function: g_surfaceChargerBMP
 * Permet de charger un fichier BMP du disque dur directement dans un type surface
 * Les tailles de la surface sont automatiquement �crite, la m�moire est automatiquement allou�e.
 * La surface est allou�e en m�moire vid�o.
 *
 * Param�tre:
 * c - chemin d'acc�s au fichier BMP
 *
 * Retour:
 * La fonction retourne un pointeur de surface correspondante � l'image BMP, si le fichier est introuvable
 * ou si la m�moire n'a pas �t� allou�e, le pointeur NULL sera retourn�.
 */
G_surface* g_surfaceChargerBMP(char* c);

/**Function g_surfaceLongueurx
 * Permet de lire la longueur d'une surface
 *
 * Param�tre:
 * s - la surface que l'on souhaite d�terminer la longueur
 *
 * Retour:
 * La longueur de la surface
 */
int g_surfaceLongueurx(G_surface* s);

/**Function g_surfaceLongueury
 * Permet de lire la hauteur d'une surface
 *
 * Param�tre:
 * s - la surface que l'on souhaite d�terminer la hauteur
 *
 * Retour:
 * La hauteur de la surface
 */
int g_surfaceLongueury(G_surface* s);

/**Function: g_surfaceEfface
 * Permet d'effacer toutes les couleurs de la surface par un rectangle noir recouvrant le tout
 *
 * Param�tre:
 * s - la surface que l'on souhaite effacer
 */
void g_surfaceEfface(G_surface* s);

/**Function: g_surfaceDetruire
 * Permet de d�truire une surface que l'on a allou� par g_surfaceCreer ou g_surfaceCharger
 *
 * Param�tre:
 * s - la surface que l'on souhaite d�truire
 */
void       g_surfaceDetruire(G_surface* surface);


/*Classes: G_fenetre
 * Ce type abstrait de donn� est la fenetre principale graphique de l'application. Elle doit �tre unique.
 * Le type n�cessite une initialisation pr�liminaire, et une destruction finale. La 
 * G_fenetre* contient
 * une surface graphique qui representera ce qui est affich� dedans.
 */

/*
 * Function: g_fenetreInitialiser
 * Permet l'initialisation d'une *G_fenetre* en fonction de resolution et d'un mode plein �cran ou non
 *
 * Param�tres: 
 * fenetre - la fenetre � initialiser
 * resolutionx - la r�solution verticale de la fenetre
 * resolutiony - la r�solution horizontale de la fenetre
 * mode - le mode plein �cran ou non plein �cran, les valeurs possibles sont: PLEIN_ECRAN et PAS_PLEIN_ECRAN
 *
 * Retour: 
 * Si la fonction a r�ussi � initialiser la fenetre, EXIT_SUCCESS est retourn�e, dans le cas contraire
 * EXIT_FAILURE est renvoy�
 */
int        g_fenetreInitialiser(G_fenetre* fenetre, int resolutionx, int resolutiony,  G_fenetre_mode mode);

/**Function: g_fenetreLireSurface
 * Permet de r�cuperer la surface graphique d'une fenetre
 *
 * Param�tres: 
 * fenetre - la fenetre que l'on souhaite recuperer la surface
 *
 * Retour:
 * Un pointeur vers la surface � recuperer
 */
G_surface* g_fenetreLireSurface(G_fenetre*);

/**Function: g_fenetreAfficher
 * Permet d'afficher la fenetre � l'�cran. Donc de mettre � jour la repr�sentation
 * Il est conseill� de la mettre � jour � chaque tour de boucle, l'utilit� de cette fonction r�side
 * dans l'utilisation du double buffering pour �viter le ph�nom�ne de clignotement
 */
void       g_fenetreAfficher(G_fenetre*);

/**Function: g_fenetreDetruire
 * Detruit la fenetre
 *
 * Param�tre:
 * fenetre - la fenetre que l'on souhaite d�truire
 */
void       g_fenetreDetruire(G_fenetre* fenetre); /*quitte tout*/


#endif

