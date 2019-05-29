CC=gcc
# INCLUDEDIR = /home/linux/include/
MODCFLAGS :=-Wall -O -DMODULE -D__KERNEL__ -DLINUX #-I$(INCLUDEDIR)
audit.o: audit.c 
	$(CC) $(MODCFLAGS) -c audit.c
