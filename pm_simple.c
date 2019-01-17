#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>        //pour stocker le majeur / mineur
#include <linux/kdev_t.h>       //pour les macro MINOR(dev_t) MAJOR(dev_t) MKDEV(int major, int minor)
#include <linux/cdev.h>
#include <linux/fs.h>


#define LICENCE "GPL"
#define AUTEUR "Zaragoza Jérémy jeremy.zaragoza@univ-tlse3.fr"
#define DESCRIPTION "Module Hello World TP1"
#define DEVICE "GPL"

int init_periph(void);
int cleanup_periph(void);
int open_periph(struct inode *str_inode, struct file *str_file);
int release_periph(struct inode *str_inode, struct file *str_file);

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
    .llseek = myLlseek,
    .read = myRead,
    .write = myWrite,
    .unlocked_ioctl = myIoctl,
    .open = &open_periph,
    .release = &release_periph
};

struct cdev *my_cdev;
dev_t first_dev;

int init_periph(void){
    /* allocation dynamique pour les paires (major,mineur) */
    if (alloc_chrdev_region(&first_dev,0,1,"sample") == -1){
        printk(KERN_ALERT "[ERROR] sample_init -> alloc_chrdev_region\n");
        return -EINVAL;
    }
    
    printk(KERN_ALERT "Init allocated (major, minor)=(%d,%d)\n",MAJOR(dev),MINOR(dev));
    /* allocation des structures pour les operations */
    my_cdev = cdev_alloc();
    my_cdev->ops = &f_operator;
    my_cdev->owner = THIS_MODULE;
    /* lien entre operations et periph */
    cdev_add(my_cdev,first_dev,1);
    return(0);
}


static void cleanup_periph(void){
    unregister_chrdev_region(first_dev,1);
    cdev_del(my_cdev);
}


int open_periph(struct inode *str_inode, struct file *str_file){
    //verif etat
    //init periph
    //indentifier mineur
    //alloc et maj données privées
    return 0;
}

int release_periph(struct inode *str_inode, struct file *str_file){
    //liberer les ref et objet alloc pendant l'open
    //desactiver le periph
    return 0;
}

static ssize_t read_periph(struct file *f, char *buffer, size_t size, loff_t *offset){
    int sizeToCopy = MIN(my_data.bufSize, size);
    printk(KERN_ALERT "Read called!\n");
    if(my_data.bufSize != 0){
        if(copy_to_user(buffer, my_data.buffer, sizeToCopy)==0){
            my_data.bufSize = 0;
            kfree(my_data.buffer);
        }
        else
            return -EFAULT;
    }
    return sizeToCopy;
}

static ssize_t sample_write(struct file *f, const char *buf, size_t size, loff_t *offset){
    printk(KERN_ALERT "Write called!\n");
    if(my_data.bufSize != 0)
        kfree(my_data.buffer);
    my_data.buffer = (char *)kmalloc(size * sizeof(char), GFP_KERNEL);
    my_data.bufSize = size - copy_from_user(my_data.buffer, buf, size);
    return my_data.bufSize;
}
