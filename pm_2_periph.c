#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>         //pour le kmalloc
#include <linux/kernel.h>
#include <linux/types.h>        //pour stocker le majeur / mineur
#include <linux/kdev_t.h>       //pour les macro MINOR(dev_t) MAJOR(dev_t) MKDEV(int major, int minor)
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>        //pour copy_from_user / copy_to_user
#include <linux/list.h>


#define LICENCE "GPL"
#define AUTEUR "Zaragoza Jérémy jeremy.zaragoza@univ-tlse3.fr"
#define DESCRIPTION "Module Peripherique TP5 lecture et ecriture dans deux peripheriques différents"
#define DEVICE "GPL"

#define DEBUG

#define MAX_BUF_SIZE 5          //pour focer a faire plusieurs nodes
#define NB_DEVICE 2

int init_periph(void);
static void cleanup_periph(void);
static int open_periph(struct inode *str_inode, struct file *str_file);
static int release_periph(struct inode *str_inode, struct file *str_file);
static ssize_t read_periph(struct file *f, char *buffer, size_t size, loff_t *offset);
static ssize_t write_periph(struct file *f, const char *buf, size_t size, loff_t *offset);

module_init(init_periph); 
module_exit(cleanup_periph);

MODULE_LICENSE(LICENCE);
MODULE_AUTHOR(AUTEUR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_SUPPORTED_DEVICE(DEVICE);

struct file_operations f_operator = {
    .owner = THIS_MODULE,
    .read = NULL,   //attribue dans le open(&read_periph,)
    .write = NULL,  //attribue dans le open (&write_periph,)
    .open = &open_periph,
    .release = &release_periph
};


/*
 * Param pour le periph
 */
struct cdev *my_cdev;
dev_t first_dev;
bool file_open = false;
//structure pour les données du periph
typedef struct s_data {
    char* buffer;
    size_t size;
    char* offset;
    struct list_head lst;
} Data;

struct list_head data_lst;

/*
 * Supprime les donnees dans data_lst
 */
void delete_data(void){
    //supression de la liste
    Data *elem;
    struct list_head *ptr;
    list_for_each(ptr, &data_lst) {
        elem = list_entry(ptr, Data, lst);
        list_del(&(elem->lst));
        kfree(elem->buffer);
        kfree(elem);
    }
}

/*
 * Initialisation du module
 */
int init_periph(void){
    /* allocation dynamique */
    // premier mineur pour l'ecriture
    // second mineur pour la lecture
    if (alloc_chrdev_region(&first_dev,0,NB_DEVICE,"sample") == -1){
        printk(KERN_ALERT "[ERROR] init_periph -> alloc_chrdev_region\n");
        return -EINVAL;
    }
    
    printk(KERN_ALERT "Init allocated (major, minor)=(%d,%d)\n",MAJOR(first_dev),MINOR(first_dev));
    
    /* allocation des structures pour les operations */
    my_cdev = cdev_alloc();
    my_cdev->ops = &f_operator;
    my_cdev->owner = THIS_MODULE;
    
    INIT_LIST_HEAD(&data_lst);
    
    /* lien entre operations et periph */
    if (cdev_add(my_cdev,first_dev,NB_DEVICE) < 0){
        printk(KERN_ALERT "[ERROR] init_periph -> cdev_add\n");
        return -EINVAL;
    }
    
    return 0;
}

/*
 * Suppression du module
 */
static void cleanup_periph(void){
    delete_data();
    list_del(&data_lst);
    
    /* liberation des majeurs / mineurs */
    unregister_chrdev_region(first_dev,NB_DEVICE);
    /* liberation du cdev */
    cdev_del(my_cdev);
#ifdef DEBUG
    printk(KERN_ALERT "[DEBUG] Desinstalle\n");
#endif
}

/*
 * Fonction utilisé lors de l'utilisation du periph
 */
static int open_periph(struct inode *str_inode, struct file *str_file){
#ifdef DEBUG
    printk(KERN_ALERT "[DEBUG] Open\n");
#endif
    //verif etat
    //init periph
    f_operator.read = NULL;
    f_operator.write = NULL;
    //indentifier mineur
    switch (iminor(str_inode)){
        //periph ecriture
        case 0 :
            f_operator.write = &write_periph;
            break;
        //periph lecture
        case 1 :
            f_operator.read = &read_periph;
            break;
        default:
            break;
    }
    //alloc et maj données privées
    return 0;
}

/*
 * Liberation du periph
 */
static int release_periph(struct inode *str_inode, struct file *str_file){
#ifdef DEBUG
    printk(KERN_ALERT "[DEBUG] Release\n");
#endif
    //liberer les ref et objet alloc pendant l'open
    //desactiver le periph
    return 0;
}

/*
 * Fonction de lecture du periph
 * -> retourne le nb d'octets lu
 */
static ssize_t read_periph(struct file *f, char *buffer, size_t size, loff_t *offset){
    size_t sizeToCopy = 0;
    Data *data;
    
    //copie des données vers l'espace utilisateur
    if(!list_empty(&data_lst)){
        data = list_first_entry(&data_lst, Data, lst);
        //recuperation taille a copier dans le buffer
        if (data->size < size)
            sizeToCopy = data->size;
        else
            sizeToCopy = size;
        
        //copy
        if(copy_to_user(buffer, data->offset, sizeToCopy)==0){
#ifdef DEBUG
            printk(KERN_ALERT "[DEBUG] Lecture de %lu octets\n",data->size);
#endif
            data->offset += sizeToCopy;
            data->size -= sizeToCopy;
            //supression de la node si tout les octets ont ete lu
            if (data->size == 0){
                list_del(&(data->lst));
                kfree(data->buffer);
                kfree(data);
            }
        }
        else
            return -EFAULT;
    }
    
    return sizeToCopy;
}

/*
 * Fonction d'ecriture du peripherique
 * -> retourne la taille restant a ecrire
 */
static ssize_t write_periph(struct file *f, const char *buf, size_t size, loff_t *offset){
    int sizeToCopy;
    Data *data;
    
    //calcul de la taille pour faire plusieurs nodes
    if (MAX_BUF_SIZE < size)
        sizeToCopy = MAX_BUF_SIZE;
    else
        sizeToCopy = size;
    
    //creation de la node
    data = (Data *)kmalloc(sizeof(struct s_data), GFP_KERNEL);
    //allocation memoire du nouveau buffer de taille "size"
    data->buffer = (char *)kmalloc(sizeToCopy * sizeof(char), GFP_KERNEL);
    data->offset = data->buffer;
    data->size = sizeToCopy - copy_from_user(data->buffer, buf, sizeToCopy);
    INIT_LIST_HEAD(&(data->lst));
    list_add_tail(&(data->lst),&data_lst);
    
#ifdef DEBUG
    printk(KERN_ALERT "[DEBUG] Ecriture de %lu octets\n",data->size);
#endif
    
    return data->size;
}
