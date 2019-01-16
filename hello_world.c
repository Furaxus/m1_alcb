#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define LICENCE "GPL"
#define AUTEUR "Zaragoza Jérémy jeremy.zaragoza@univ-tlse3.fr"
#define DESCRIPTION "Module Hello World TP1"
#define DEVICE "GPL"


/* Définition d'un paramètre                                                                     */
/*      Macro MODULE_PARAM()                                                                      */
/*           Nom du paramètre                                                                    */
/*           Type du paramètre : b (byte), h (short integer), i (integer), l (long integer),     */
/*                               s (string - allocation par insmod)                              */
/*      Macro MODULE_PARM_DESC()                                                                 */
/*           Nom du paramètre                                                                    */
/*           Description (chaîne de caractères)                                                  */


char *Message="HelloWorld";
module_param(Message, charp, S_IRUGO);
MODULE_PARM_DESC(Message,"Message affiche a l'insertion du module");

char *Utilisateur="Jérémy Zaragoza";
module_param(Utilisateur, charp, S_IRUGO);
MODULE_PARM_DESC(Utilisateur,"Utilisateur a afficher lors de la supression du module");

static int hello_init(void){
  printk(KERN_ALERT "%s!\n",Message);
  return 0;
}

static void hello_cleanup(void){
  printk(KERN_ALERT "Aurevoir %s\n",Utilisateur);
}

module_init(hello_init); 
module_exit(hello_cleanup); 


/* Types de licences supportées :                                                                */
/*      "GPL"                        GNU Public Licence V2 ou ultérieure                         */
/*      "GPL v2"                     GNU Public Licence v2                                       */
/*      "GPL and additional rights   GNU Public Licence v2 et droits complémentaires             */
/*      "Dual BSD/GPL"               Licence GPL ou BSD au choix                                 */
/*      "Dual MPL/GPL"               Licence GPL ou Mozilla au choix                             */
/*      "Propietary"                 Produit à diffusion non libre (commercial)                  */

MODULE_LICENSE(LICENCE);
MODULE_AUTHOR(AUTEUR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_SUPPORTED_DEVICE(DEVICE);
