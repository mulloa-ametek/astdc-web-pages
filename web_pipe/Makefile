TOOLSDIR ?= ~/release/linux-cortexm-1.12.0/tools/arm-2010q1/arm-uclinuxeabi
CC := $(CROSS_COMPILE)gcc
LN := $(CC)
CFLAGS := -Os -mcpu=cortex-m3 -mthumb -I$(INSTALL_ROOT)/A2F/root/usr/include 
LNFLAGS := -mcpu=cortex-m3 -mthumb -L$(INSTALL_ROOT)/A2F/root/usr/lib -pthread

.PHONY: clean all

all: web_pipe test_client

test_client: test_client.o
	$(LN) $(LNFLAGS) $< -o $@

test_client.o: test_client.c
	$(CC) $(CFLAGS) -c $<

web_pipe: web_pipe.o
	$(LN) $(LNFLAGS) $< -o $@

web_pipe.o: web_pipe.c
	$(CC) $(CFLAGS) -c $<

clean:
	@echo 'check_password: Cleaning up'
	@if [ -e web_pipe ]; then rm web_pipe; fi
	@if [ -e web_pipe.gdb ]; then rm web_pipe.gdb; fi
