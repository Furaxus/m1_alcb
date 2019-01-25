CONFIG_MODULE_SIG=n

ifneq ($(KERNELRELEASE),)
	obj-m := pm_simple.o
else
	KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C ${KERNEL_DIR} M=$(PWD) modules
endif
