CC=gcc
CFLAGS=-g
all: server rget

common.o: common.c
	${CC} ${CFLAGS} -c common.c

libsocket.o: libsocket.c
	${CC} ${CFLAGS} -c libsocket.c

server.o: server.c
	${CC} ${CFLAGS} -c server.c

server: common.o libsocket.o server.o
	${CC} server.o libsocket.o common.o -o server

rget.o: rget.c
	${CC} ${CFLAGS} -c rget.c

rget: common.o libsocket.o rget.o
	${CC} rget.o common.o libsocket.o -o rget

clean:
	rm -rf *.o
	rm -rf server
	rm -rf rget
