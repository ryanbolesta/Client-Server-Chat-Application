# Makefile for TCP client/server application

OS := $(shell uname -s)
ifeq ($(OS), Linux)
	LIBS = -lnsl
endif
ifeq ($(OS), SunOS)
	LIBS = -lnsl -lsocket
endif


all: server client

server: server.c utils.o
		gcc -o server server.c utils.o -pthread $(LIBS)

client: client.c utils.o
		gcc -o client client.c utils.o $(LIBS)


utils.o: utils.c
	gcc -c utils.c

clean:
	/bin/rm -f server client
	/bin/rm -f *.o
