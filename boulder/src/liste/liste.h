#ifndef _LISTE_H
#define _LISTE_H

/* Topic: Documentation du module Liste
 * Module permettant l'utilisation d'une liste g�n�ralis�e � tous les types.
 * 
 * Une liste doit toujours �tre initialis�e avant l'utilisation de celle-ci.
 * Lorsque l'utilisateur n'en a plus besoin, il faut la d�truire par lib�rer l'espace
 * allou� par celle-ci.
 */

typedef void ElementListe;


typedef struct Liste_cellule_ {
        ElementListe* element;
        struct Liste_cellule_ * suivant;
        struct Liste_cellule_ * precedent;
        } Liste_cellule;
        
typedef struct Liste_ {
        Liste_cellule* premier;
        Liste_cellule* dernier;
        int            taille;
        Liste_cellule* derniere_cellule_lue;
        int            derniere_position;
               
        } Liste;
        
/*constructeur et destructeur*/
/**Function: listeInitialiser
 * Initialisation d'une liste
 *
 * Param�tres:
 * liste - liste devant �tre initialis�e
 */
void listeInitialiser (Liste* liste);

/**Function: listeDetruire
 * Destruction de toute la liste � partir de son pointeur
 *
 * Param�tres
 * liste - liste devant �tre d�truite
 */
void listeDetruire (Liste* liste);

/*modificateurs d'�tats*/

/**Function: listeAjouter
 * Permet d'ajouter un pointeur vers un �l�ment � une liste
 *
 * Param�tres
 * liste - la liste ou doit �tre ajout� l'�l�ment
 * position - la position ou ajouter l'�l�ment
 * element - le pointeur vers l'�l�ment � ajouter
 *
 * Erreur:
 *  Dans le cas d'une limite m�moire, un message d'erreur sera affich� sur la sortie stderr
 *  Dans le cas d'une tentative d'ajout hors borne, un message d'erreur est affich� sur la sortie stderr
 */
void listeAjouter (Liste* liste, int position, ElementListe* element);

/**Function: listeRetirer
 * Permet de retirer un �l�ment d'une liste
 *
 * Param�tres
 * liste - la liste ou doit �tre retir� l'�l�ment
 * position - la position ou retirer l'�l�ment
 *
 * Erreur:
 *  Dans le cas d'une tentative d'ajout hors borne, un message d'erreur est affich� sur la sortie stderr
 */
void listeRetirer (Liste* liste, int position);

/**Function: listeAjouterFin
 * Permet d'ajouter un �l�ment � la fin d'un fichier
 *
 * Param�tres
 * liste - la liste ou doit �tre ajout�l'�l�ment
 * element - l'�l�ment � ajouter
 *
 * Erreur:
 *  Dans le cas d'un probl�me m�moire, un message d'erreur est affich� sur la sortie stderr
 */
void listeAjouterFin(Liste* liste, ElementListe* element);

void listeEcrire(Liste* liste, int position, ElementListe* element);

/*accesseurs*/
/**Function: listeLire
 * Permet de recuperer un �l�ment d'une liste
 *
 * Param�tre:
 * liste - la liste ou l'on doit lire l'�l�ment
 * position - position de l'�l�ment
 *
 * Retour: 
 * Un pointeur vers l'�l�ment lu
 *
 * Erreur:
 *  Dans le cas d'une tentative d'�criture hors borne, le pointeur NULL est retourn�
 */
ElementListe* listeLire (Liste* liste, int position);

/**Function: listeTaille
 *  D�termine la taille d'une liste
 * Param�tre:
 * liste - liste � d�terminer la taille
 *
 * Retour:
 *  Le taille de la liste
 */
int listeTaille (Liste* liste);

int listeEstVide (Liste* liste);
//ElementListe* listeLire(Liste* liste, int position);
#endif
