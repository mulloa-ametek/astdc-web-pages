#######################################################################################
# Title:					Makefile
# Author:				Adam Mocarski
# Description:			Makefile for CGI implementation of the configuration web page
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

all: configuration.cgi

configuration.cgi: configuration.o $(CGI_DIRECTORY)/libcgi.a
	$(LN) $(LNFLAGS) $^ -o $@ -L$(CGI_DIRECTORY) -lcgi

configuration.o: configuration.c ../cgi-lib/cgi-lib.h
	$(CC) $(CFLAGS) -c $<

clean:
	@echo Cleaning up `pwd`
	@if [ -e configuration.cgi ]; then rm configuration.cgi; fi
	@if [ -e configuration.cgi.gdb ]; then rm configuration.cgi.gdb; fi
	@if [ -e configuration.o ]; then rm configuration.o; fi
