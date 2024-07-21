obj-m += dtsofled.o
# obj-m += ../common/dbgmsg.o
#generate the path
CURRENT_PATH:=$(shell pwd)
#the current kernel version number
LINUX_KERNEL:=$(shell uname -r)
#the absolute path
LINUX_KERNEL_PATH:=/usr/src/linux-headers-$(LINUX_KERNEL)

#complie object
all:
	echo make -C $(LINUX_KERNEL_PATH) $(EXTRA_CFLAGS) M=$(CURRENT_PATH) modules
	make -C $(LINUX_KERNEL_PATH) $(EXTRA_CFLAGS) M=$(CURRENT_PATH) modules
#clean
clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
