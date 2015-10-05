#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = server-mt.o request.o util.o client.o
TARGET = server-mt 

CC = gcc
CFLAGS = -g -Wall

LIBS = -lpthread 

.SUFFIXES: .c .o 

all: server-mt client output.cgi

server-mt: server-mt.o request.o util.o
	$(CC) $(CFLAGS) -o server-mt server-mt.o request.o util.o $(LIBS)

client: client.o util.o
	$(CC) $(CFLAGS) -o client client.o util.o

output.cgi: output.c
	$(CC) $(CFLAGS) -o output.cgi output.c

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) server-mt client output.cgi
