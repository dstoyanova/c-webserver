#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = server-st.o request.o util.o client.o
TARGET = server-st

CC = gcc
CFLAGS = -g -Wall

.SUFFIXES: .c .o 

all: server-st client

server-st: server-st.o request.o util.o
	$(CC) $(CFLAGS) -o server-st server-st.o request.o util.o

client: client.o util.o
	$(CC) $(CFLAGS) -o client client.o util.o

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) server-st client