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
#define DESCRIPTION "Module Peripherique TP2 simple"
#define DEVICE "GPL"

#define BUF_SIZE 4            //taille du buffer

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
                          
/*
 * loff_t (*llseek) (struct file *, loff_t, int);
 * ssize_t (*read) (struct file *, char *, size_t, loff_t *);
 * ssize_t (*write) (struct file *, const char *, size_t, loff_t *);
 * int (*readdir) (struct file *, void *, filldir_t);
 * unsigned int (*poll) (struct file *, struct poll_table_struct *);
 * int (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
 * int (*mmap) (struct file *, struct vm_area_struct *);
 * int (*open) (struct inode *, struct file *);
 * int (*flush) (struct file *);
 * int (*release) (struct inode *, struct file *);
 * int (*fsync) (struct inode *, struct dentry *, int);
 * int (*lock) (struct file *, int, struct file_lock *);
 * ssize_t (*readv) (struct file *, const struct iovec *, unsigned long, loff_t *);
 * ssize_t (*writev) (struct file *, const struct iovec *, unsigned long, loff_t *);
 * struct module *owner;
 */
                                
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
    bool mult_write;
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
    
    printk(KERN_ALERT "[DEBUG] Init allocated (major, minor)=(%d,%d)\n",MAJOR(first_dev),MINOR(first_dev));
    
    /* allocation des structures pour les operations */
    my_cdev = cdev_alloc();
    my_cdev->ops = &f_operator;
    my_cdev->owner = THIS_MODULE;
    
    data.mult_write = false;
    
    /* lien entre operations et periph */
    if (cdev_add(my_cdev,first_dev,1) < 0){
        printk(KERN_ALERT "[ERROR] init_periph -> cdev_add\n");
        return -EINVAL;
    }
    
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
    printk(KERN_ALERT "[DEBUG] Desinstalle\n");
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
    //fin des ecritures en serie
    //data.mult_write = false;
    
    //liberer les ref et objet alloc pendant l'open
    //desactiver le periph
    return 0;
}

/*
 * Fonction de lecture du periph
 * -> retourne la taille lu
 */
static ssize_t read_periph(struct file *f, char *buffer, size_t size, loff_t *offset){
    //recuperation taille a copier dans le buffer
    int sizeToCopy;
    if (BUF_SIZE < size)
        sizeToCopy = BUF_SIZE;
    else
        sizeToCopy = size;
    
    //copie des données vers l'espace utilisateur
    if(data.buffer != NULL){
        if(copy_to_user(buffer, data.buffer, sizeToCopy)==0){
            printk(KERN_ALERT "[DEBUG] Lecture : %s\n",data.buffer);
            kfree(data.buffer);     //lecture destructrice
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
    //on vide le buffer avant utilisation (ecriture destructrice)
    if(data.buffer != NULL/* && !data.mult_write */)
        kfree(data.buffer);
    
    //allocation memoire du nouveau buffer de taille "BUF_SIZE"
    data.buffer = (char *)kmalloc(BUF_SIZE * sizeof(char), GFP_KERNEL);
    
    //recuperation des données depuis l'espace utilisateur
    int sizeCopy = copy_from_user(data.buffer, buf, size);
    //on indique qu'on viens de faire une ecriture
    //data.mult_write = true;
    
    printk(KERN_ALERT "[DEBUG] Ecriture : %s\n",data.buffer);
    
    return (size - sizeCopy);
}
