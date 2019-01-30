tp_files := hello_world.o pm_simple.o pm_taille_variable.o

ifneq ($(KERNELRELEASE),)
	obj-m := $(tp_files)
else
	KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C ${KERNEL_DIR} M=$(PWD) modules
	
clean:
	rm -rf $(tp_files) *.ko *.mod.c *.mod.o modules.order Module.symvers
endif



#copy de fichier :
#scp pm_simple.c root@vm-dyn-0-203:/root/tp1

#installation
#mknod [nom_du_module] c [majeur] [mineur]

#test ecriture / lecture 
#echo [some random text] > /dev/[nom_du_module]

#instalation du module
#insmod [nom_du_module].ko

#suppression du module
#rmmod [nom_du_module].ko
