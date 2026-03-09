#######################################################################################
# Title:					Makefile
# Author:				Adam Mocarski
# Description:			Makefile for CGI implementation of the update page
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

all: cgi_update.cgi

cgi_update.cgi: cgi_update.o $(CGI_DIRECTORY)/libcgi.a
	$(LN) $(LNFLAGS) $^ -o $@ -L$(CGI_DIRECTORY) -lcgi

cgi_update.o: cgi_update.c ../cgi-lib/cgi-lib.h ../cgi-lib/libcgi.a
	$(CC) $(CFLAGS) -c $<

clean:
	@echo Cleaning up `pwd`
	@if [ -e cgi_update.cgi ]; then rm cgi_update.cgi; fi
	@if [ -e cgi_update.cgi.gdb ]; then rm cgi_update.cgi.gdb; fi
	@if [ -e cgi_update.o ]; then rm cgi_update.o; fi
