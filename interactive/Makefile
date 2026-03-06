#######################################################################################
# Title:					Makefile
# Author:				Adam Mocarski
# Description:			Makefile for CGI implementation of the interactive page
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

all: interactive.cgi

interactive.cgi: interactive.o $(CGI_DIRECTORY)/libcgi.a
	$(LN) $(LNFLAGS) $^ -o $@ -L$(CGI_DIRECTORY) -lcgi

interactive.o: interactive.c ../cgi-lib/cgi-lib.h ../cgi-lib/libcgi.a
	$(CC) $(CFLAGS) -c $<

clean:
	@echo Cleaning up `pwd`
	@if [ -e interactive.cgi ]; then rm interactive.cgi; fi
	@if [ -e interactive.cgi.gdb ]; then rm interactive.cgi.gdb; fi
	@if [ -e interactive.o ]; then rm interactive.o; fi
