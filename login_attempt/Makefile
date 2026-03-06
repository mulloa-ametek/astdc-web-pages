TOOLSDIR ?= ~/release/linux-cortexm-1.12.0/tools/arm-2010q1/arm-uclinuxeabi
CC := $(CROSS_COMPILE)gcc
LN := $(CC)
CFLAGS := -Os -mcpu=cortex-m3 -mthumb -I$(INSTALL_ROOT)/A2F/root/usr/include 
LNFLAGS := -mcpu=cortex-m3 -mthumb -L$(INSTALL_ROOT)/A2F/root/usr/lib

CGI_DIRECTORY := ../cgi-lib

.PHONY: all clean

all: login_attempt

login_attempt: check_password.o login_attempt.o $(CGI_DIRECTORY)/libcgi.a
	$(LN) $(LNFLAGS) $^ -o $@ -L$(CGI_DIRECTORY) -lcgi

check_password.o: check_password.c check_password.h
	$(CC) $(CFLAGS) -c $<

login_attempt.o: login_attempt.c check_password.h ../cgi-lib/cgi-lib.h
	$(CC) $(CFLAGS) -c $<

clean:
	@echo Cleaning up `pwd`
	@if [ -e login_attempt ]; then rm login_attempt; fi
	@if [ -e login_attempt.gdb ]; then rm login_attempt.gdb; fi
	@if [ -e check_password.o ]; then rm check_password.o; fi
	@if [ -e login_attempt.o ]; then rm login_attempt.o; fi
