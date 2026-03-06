##############################################################################
# Title:				Makefile
# AUthor:			Adam Mocarski
# Description:		Makefile for executables and files needed by the website
#
# Copyright (C) AMETEK Programmable Power, 2014
##############################################################################
# library for handling basic CGI calls
CGI_LIB := cgi-lib
# projects that depend
CGI_SUBPROJECTS := login_attempt configuration blink interactive update
# projects that don't depend on the CGI libraries
NON_CGI_SUBPROJECTS := web_pipe module_list

.PHONY: clean all $(CGI_LIB) $(CGI_SUBPROJECTS) $(NON_CGI_SUBPROJECTS)

all: $(CGI_SUBPROJECTS) $(NON_CGI_SUBPROJECTS)

$(CGI_SUBPROJECTS): $(CGI_LIB)
	make -C $@

$(NON_CGI_SUBPROJECTS):
	make -C $@

$(CGI_LIB):
	make -C $(CGI_LIB)
clean:
	for i in $(CGI_SUBPROJECTS); do make -C $$i clean; done
	for i in $(NON_CGI_SUBPROJECTS); do make -C $$i clean; done
	for i in $(CGI_LIB); do make -C $$i clean; done
