TOOLDIR = /opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux
ARCH = x86
CROSS_COMPILE = /opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux/i586-poky-linux-

ifeq	($(TEST_TARGET), Galileo2)
	CC = $(TOOLDIR)/i586-poky-linux-gcc
	MAKE = make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
	KDIR=/opt/iot-devkit/1.7.2/sysroots/i586-poky-linux/usr/src/kernel
else
	CC=gcc
	MAKE=make
	KDIR=/lib/modules/$(shell uname -r)/build
endif


APP=user_prog

obj-m:= squeue.o

all: User_prog.c Queue.h rdtsc.h
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	$(CC) -Wall -o $(APP) User_prog.c -pthread -lm 

clean:
	rm -f *.ko
	rm -f *.o
	rm -f Module.symvers
	rm -f modules.order
	rm -f *.mod.c
	rm -rf .tmp_versions
	rm -f *.mod.c
	rm -f *.mod.o
	rm -f \.*.cmd
	rm -f Module.markers
	rm -f $(APP) 
