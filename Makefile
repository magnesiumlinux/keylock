CC=gcc
CFLAGS=-Wall
LDFLAGS=-static

keylock: cmd.o client.o server.o mote.o ceylock.o
	$(CC) $^ -static -o keylock 

clean:
	rm -rf *.o keylock

ceylock.o: ceylock.c ceylock.h
mote.o: mote.c mote.h ceylock.h
client.o: client.c ceylock.h
server.o: server.c ceylock.h mote.h
cmd.o: cmd.c ceylock.h
