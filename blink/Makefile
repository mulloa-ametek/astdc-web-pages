#######################################################################################
# Title:					Makefile
# Author:				Adam Mocarski
# Description:			Makefile for CGI implementation of the lxi identification blink
#
# Copyright (C) AMETEK programmable Power, 2014
#
#######################################################################################

TOOLSDIR ?= ~/release/linux-cortexm-1.12.0/tools/arm-2010q1/arm-uclinuxeabi
CC := $(CROSS_COMPILE)gcc
LN := $(CC)
CFLAGS := -Os -mcpu=cortex-m3 -mthumb -I$(INSTALL_ROOT)/A2F/root/usr/include 
LNFLAGS := -mcpu=cortex-m3 -mthumb -L$(INSTALL_ROOT)/A2F/root/usr/lib

CGI_DIRECTORY := ../cgi-lib

.PHONY: all clean

all: blink.cgi

blink.cgi: blink.o $(CGI_DIRECTORY)/libcgi.a
	$(LN) $(LNFLAGS) $^ -o $@ -L$(CGI_DIRECTORY) -lcgi

blink.o: blink.c ../cgi-lib/cgi-lib.h ../cgi-lib/libcgi.a
	$(CC) $(CFLAGS) -c $<

clean:
	@echo Cleaning up `pwd`
	@if [ -e blink.cgi ]; then rm blink.cgi; fi
	@if [ -e blink.cgi.gdb ]; then rm blink.cgi.gdb; fi
	@if [ -e blink.o ]; then rm blink.o; fi
