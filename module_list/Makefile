#######################################################################################
# Title:					Makefile
# Author:				Adam Mocarski
# Description:			Makefile for CGI implementation of the module_page
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

all: module_list.cgi

module_list.cgi: module_list.o $(CGI_DIRECTORY)/libcgi.a
	$(LN) $(LNFLAGS) $^ -o $@ -L$(CGI_DIRECTORY) -lcgi

module_list.o: module_list.c ../cgi-lib/cgi-lib.h ../cgi-lib/libcgi.a
	$(CC) $(CFLAGS) -c $<

clean:
	@echo Cleaning up `pwd`
	@if [ -e module_list.cgi ]; then rm module_list.cgi; fi
	@if [ -e module_list.cgi.gdb ]; then rm module_list.cgi.gdb; fi
	@if [ -e module_list.o ]; then rm module_list.o; fi
