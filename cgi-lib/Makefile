################################################################################
# Makefile (native GCC on Ubuntu) for cgi-lib
# Builds a host-compatible static library: libcgi.a
################################################################################

# Tools (native toolchain)
CC      ?= gcc
AR      ?= ar

# Flags
CSTD     ?= -std=c11
WARN     ?= -Wall -Wextra -Wpedantic
OPT      ?= -O2
CPPFLAGS ?= -I.
CFLAGS   ?= $(CSTD) $(WARN) $(OPT)
ARFLAGS  ?= rcs

# Outputs
LIB      := libcgi.a
OBJS     := cgi-lib.o

.PHONY: all clean test

all: $(LIB)

# Build the static library
$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

# Object(s)
cgi-lib.o: cgi-lib.c cgi-lib.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Optional: small test program that links against libcgi.a
test: $(LIB) test.o
	$(CC) -o $@ test.o -L. -lcgi

test.o: test.c cgi-lib.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning $(PWD)"
	@rm -f $(OBJS) $(LIB) test.o test
