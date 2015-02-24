ifneq ($(KERNELRELEASE),)
    obj-m := invaderModule.o 
    invaderModule-objs := module.o
	
else
    KDIR    := /lib/modules/$(shell uname -r)/build
    PWD    := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	g++ -o main main.cpp ioctl_cmds.h
	
clean:
	${MAKE} -C ${KDIR} SUBDIRS=${PWD} clean
	rm -rf main *.o

load:
	shellscripts/load.sh invaderModule format

unload:
	shellscripts/unload.sh invaderModule

test:
	./main
	
endif
