TOOLSDIR ?= ~/release/linux-cortexm-1.12.0/tools/arm-2010q1/arm-uclinuxeabi
CC := $(CROSS_COMPILE)gcc
LN := $(CC)
CFLAGS := -Os -mcpu=cortex-m3 -mthumb -I$(INSTALL_ROOT)/A2F/root/usr/include 
LNFLAGS := -mcpu=cortex-m3 -mthumb -L$(INSTALL_ROOT)/A2F/root/usr/lib

.PHONY: clean all

all: check_password

check_password: check_password.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	@echo 'check_password: Cleaning up'
	@if [ -e check_password ]; then rm check_password; fi
	@if [ -e check_password.gdb ]; then rm check_password.gdb; fi
