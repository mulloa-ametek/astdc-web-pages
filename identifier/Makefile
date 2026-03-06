TOOLSDIR ?= ~/release/linux-cortexm-1.12.0/tools/arm-2010q1/arm-uclinuxeabi
CC := $(CROSS_COMPILE)gcc
LN := $(CC)
CFLAGS := -Os -mcpu=cortex-m3 -mthumb -I$(INSTALL_ROOT)/A2F/root/usr/include 
LNFLAGS := -mcpu=cortex-m3 -mthumb -L$(INSTALL_ROOT)/A2F/root/usr/lib

lxi_identification: identifier.o lxi_identification.o
	$(LN) $(LNFLAGS) $^ -o $@

identifier.o: identifier.c lxi_identification.h
	$(CC) $(CFLAGS) -c $<

lxi_identification.o: lxi_identification.c lxi_identification.h
	$(CC) $(CFLAGS) -c $<
