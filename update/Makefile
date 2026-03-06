#######################################################################################
# Title:          Makefile (native GCC on Ubuntu)
# Description:    Build update.cgi against ../cgi-lib on the host (no cross-compile)
#######################################################################################

# Compiler & Linker
CC      = gcc
LD      = gcc

# Directories
CGI_DIR = ../cgi-lib

# Flags
CFLAGS  = -std=c11 -Wall -Wextra -O2 -I$(CGI_DIR)
LDFLAGS =
LIBS    = -L$(CGI_DIR) -lcgi

# Targets
TARGET  = update.cgi
OBJS    = update.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) $(CGI_DIR)/libcgi.a
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

update.o: update.c $(CGI_DIR)/cgi-lib.h
	$(CC) $(CFLAGS) -c update.c -o update.o

clean:
	rm -f $(TARGET) update.o update.cgi.gdb
