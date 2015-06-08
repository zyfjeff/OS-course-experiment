CC=gcc
CFLAGS=-g -Wall -O2
all: sfindv2 sfind

sfindv2: sfindv2.o
	${CC} -o sfindv2 sfindv2.o

sfindv2.o: sfindv2.c
	${CC} ${FLAGS} -c sfindv2.c

sfind: sfind.o
	${CC} -o sfind sfind.o

sfind.o: sfind.c
	${CC} ${FLAGS} -c sfind.c

clean:
	rm -rf *.o
	rm -rf sfindv2
	rm -rf sfind
