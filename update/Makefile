CC ?= gcc
CFLAGS ?= -O2 -Wall

all: upload.cgi

upload.cgi: upload.o
	$(CC) $(CFLAGS) -o $@ upload.o

upload.o: upload.c
	$(CC) $(CFLAGS) -c upload.c

clean:
	rm -f upload.o upload.cgi