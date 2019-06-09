obj-m +=task_active_pid_ns.o
obj-m += ../common/dbgmsg.o
#generate the path
CURRENT_PATH:=$(shell pwd)
#the current kernel version number
LINUX_KERNEL:=$(shell uname -r)
#the absolute path
LINUX_KERNEL_PATH:=/usr/src/linux-headers-$(LINUX_KERNEL)

KONAME=`basename $(PWD)`.ko
all:compile show
	@echo all done!
clean:doclean show
	@echo clean done!
#complie object
compile:
	@echo make -C $(LINUX_KERNEL_PATH) $(EXTRA_CFLAGS) M=$(CURRENT_PATH) modules
	make -C $(LINUX_KERNEL_PATH) $(EXTRA_CFLAGS) M=$(CURRENT_PATH) modules
	@echo koname is $(KONAME)
	sudo insmod $(KONAME)
#clean
doclean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
	sudo rmmod $(KONAME)

show:
	@sudo dmesg -c -T
