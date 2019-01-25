#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>        //pour le kmalloc
#include <linux/kernel.h>
#include <linux/types.h>        //pour stocker le majeur / mineur
#include <linux/kdev_t.h>       //pour les macro MINOR(dev_t) MAJOR(dev_t) MKDEV(int major, int minor)
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>        //pour copy_from_user / copy_to_user


#define LICENCE "GPL"
#define AUTEUR "Zaragoza Jérémy jeremy.zaragoza@univ-tlse3.fr"
#define DESCRIPTION "Module Peripherique TP3 taille variable"
#define DEVICE "GPL"

#define MAX_BUF_SIZE 256

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
    .read = &read_periph,
    .write = &write_periph,
    .open = &open_periph,
    .release = &release_periph
};


/*
 * Param pour le periph
 */
struct cdev *my_cdev;
dev_t first_dev;
//structure pour les données du periph
typedef struct {
    char* buffer;
    size_t size;
} Data;

static Data data;

/*
 * Initialisation du module
 */
int init_periph(void){
    /* allocation dynamique */
    if (alloc_chrdev_region(&first_dev,0,1,"sample") == -1){
        printk(KERN_ALERT "[ERROR] init_periph -> alloc_chrdev_region\n");
        return -EINVAL;
    }
    
    printk(KERN_ALERT "Init allocated (major, minor)=(%d,%d)\n",MAJOR(first_dev),MINOR(first_dev));
    
    /* allocation des structures pour les operations */
    my_cdev = cdev_alloc();
    my_cdev->ops = &f_operator;
    my_cdev->owner = THIS_MODULE;
    
    /* lien entre operations et periph */
    if (cdev_add(my_cdev,first_dev,1) < 0){
        printk(KERN_ALERT "[ERROR] init_periph -> cdev_add\n");
        return -EINVAL;
    }
    //data.buffer = (char *)kmalloc(BUF_SIZE * sizeof(char), GFP_KERNEL);
    data.size = 0;
    
    return 0;
}

/*
 * Suppression du module
 */
static void cleanup_periph(void){
    if (data.buffer != NULL)
        kfree(data.buffer);
    /* liberation des majeurs / mineurs */
    unregister_chrdev_region(first_dev,1);
    /* liberation du cdev */
    cdev_del(my_cdev);
}

/*
 * Fonction utilisé lors de l'utilisation du periph
 */
static int open_periph(struct inode *str_inode, struct file *str_file){
    //verif etat
    //init periph
    //indentifier mineur
    //alloc et maj données privées
    return 0;
}

/*
 * Liberation du periph
 */
static int release_periph(struct inode *str_inode, struct file *str_file){
    //liberer les ref et objet alloc pendant l'open
    //desactiver le periph
    return 0;
}

/*
 * Fonction de lecture du periph
 * -> retourne la taille lu
 */
static ssize_t read_periph(struct file *f, char *buffer, size_t size, loff_t *offset){
    printk(KERN_ALERT "[DEBUG] Lecture\n");
    
    //recuperation taille a copier dans le buffer
    int sizeToCopy;
    if (MAX_BUF_SIZE < size)
        sizeToCopy = MAX_BUF_SIZE;
    else
        sizeToCopy = size;
    
    //copie des données vers l'espace utilisateur
    if(data.size != 0){
        if(copy_to_user(buffer, data.buffer, sizeToCopy)==0)
            kfree(data.buffer);     //lecture destructrice
            data.size = 0;          //buffer vide
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
    printk(KERN_ALERT "[DEBUG] Ecriture\n");
    //on vide le buffer avant utilisation (ecriture destructrice)
    if(data.size != 0){
        kfree(data.buffer);
        data.size = 0;
    }
    
    //allocation memoire du nouveau buffer de taille "BUF_SIZE"
    data.buffer = (char *)kmalloc(size * sizeof(char), GFP_KERNEL);
    
    //recuperation des données depuis l'espace utilisateur
    data.size = copy_from_user(data.buffer, buf, size);
    
    return (size - data.size);
}
