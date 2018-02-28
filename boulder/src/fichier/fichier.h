#ifndef _FICHIER_H
#define _FICHIER_H
#define FIN_DE_FICHIER EOF
/* Topic: Documentation du module Fichier
 * Ce module permet la gestion de flux de fichier en lecture
 * en utilisant certaines op�rations d�finies.
 */
 
typedef FILE Fichierl;

/**Function: fichierlOuvrir
 * Fonction permettant l'ouverture d'un fichier par rapport � son chemin d'acc�s
 * 
 * Param�tre:
 * c - chemin d'acc�s au fichier
 *
 * Retour: 
 *  Pointeur vers un fichierl
 *
 * Erreur:
 *  Si on ne trouve pas le fichier sur le disque dur, le pointeur NULL est renvoy�
 */
Fichierl* fichierlOuvrir(char* c);

/**Function: fichierlLireEntier
 * Fonction permettant de lire un entier dans un flux de type *Fichierl*
 *
 * Param�tres:
 * f - fichier � lire
 * i - l'entier lu sera �crit dans i
 *
 * Retour:
 *  Si l'entier a �t� lu, la fonction retourne EXIT_SUCCESS
 *
 * Erreur:
 *  Si la fin de fichier a �t� atteint avant d'obtenir un entier, FIN_DE_FICHIER est retourn�
 */
int fichierlLireEntier (Fichierl* f, int* i);

/**Function: fichierlLireCaractere
 * Fonction permettant de lire un caract�re dans un flux de type *Fichierl*. Les caract�res sp�ciaux
 * tel que \n ou \0 sont ignor�s
 * Param�tres:
 * f - fichier � lire
 * lu - le caract�re lu sera �crit dans i
 *
 * Retour:
 *  Si l'entier a �t� lu, la fonction retourne EXIT_SUCCESS
 *
 * Erreur:
 *  Si la fin de fichier a �t� atteint avant d'obtenir un caract�re, FIN_DE_FICHIER est retourn�
 */
int fichierlLireCaractere (Fichierl*f, char* lu);

/**Function: fichierlLireLigne
 * Fonction permettant de lire une ligne de caract�res dans un flux de type *Fichierl*. Les caract�res sp�ciaux
 * tel que \n ou \0 sont ignor�s
 * Param�tres:
 * f - fichier � lire
 * lu - on �crira dans cette chaine
 *
 * Retour:
 *  Si on a bien obtenu un caract�re, la fonction retourne EXIT_SUCCESS
 *
 * Erreur:
 *  Si la fin de fichier a �t� atteint avant d'obtenir un caract�re, FIN_DE_FICHIER est retourn�.
 *  Si le fichier pointe sur NULL, EXIT_FAILURE est retourn�
 */
int fichierlLireLigne (Fichierl* f, char* c);

/**Function: fichierlLireEgale
 * Fonction permettant de lire une ligne de caract�res dans un flux de type *Fichierl* jusqu'� ce que le symbole = est rencontr�. Les caract�res sp�ciaux
 * tel que \n ou \0 sont ignor�s
 * Param�tres:
 * f - fichier � lire
 * lu - on �crira dans cette chaine
 *
 * Retour:
 *  Si il n'existe plus de ligne, la fonction retourne EXIT_SUCCESS.
 *  c a �t� modifi� tel que son dernier caract�re soit \0
 *
 * Erreur:
 *  Si la fin de fichier a �t� atteint avant d'obtenir un caract�re, FIN_DE_FICHIER est retourn�.
 *  Si le fichier pointe sur NULL, EXIT_FAILURE est retourn�
 */
int fichierlLireEgale (Fichierl* f, char* c);

/**Function: fichierlFermer
 * Fonction permettant la fermeture d'un *fichierl* 
 * 
 * Param�tre:
 * f - flux de fichier
 *
 * Retour: 
 *  EXIT_SUCCESS si on a bien r�ussi � fermer le fichier
 *
 * Erreur:
 *  Dans le cas d'un probl�me, EXIT_FAILURE est retourn�.
 */
int fichierlFermer(Fichierl *f);

#endif
