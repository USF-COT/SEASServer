#############################################################################
#
#	Makefile for building the char-driver sample module
#
#############################################################################

ifeq ($(DEBUG),y)
	CFLAGS += -O -g		# -O is need to expand inlines
else
	CFLAGS += -O2
endif

ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m	:= usleep-drv.o

else

PWD     := $(shell pwd)

GUMSTIX_BUILDROOT	?= $(PWD)/../gumstix-oe
KERNELDIR 		?= $(GUMSTIX_BUILDROOT)/tmp/staging/gumstix-custom-verdex-angstrom-linux-gnueabi/kernel
CROSS_COMPILE		?= $(GUMSTIX_BUILDROOT)/tmp/cross/arm-angstrom-linux-gnueabi/bin/

default: user-apps kernel-module

.PHONY: user-apps kernel-module

user-apps: usleep-test

TARGET_ARCH=-Os -march=armv5te -mtune=xscale -Wa,-mcpu=xscale
CC = $(CROSS_COMPILE)gcc

CPPFLAGS	+= 
CFLAGS		+= -Wall

OBJS = usleep-test.o

usleep-test: $(OBJS)

kernel-module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) modules

endif

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers usleep-test

depend .depend dep:
	@echo "Creating dependencies ..."
	$(CC) $(CFLAGS) -M *.c $(SHARED)/*.c > .depend
								 
ifeq (.depend,$(wildcard .depend))
include .depend
endif
