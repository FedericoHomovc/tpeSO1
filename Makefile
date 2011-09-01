#
#
#	Makefile for tpeSO
#		Jose Ignacio Galindo
#		Federico Homovc
#		Nicolas Loreti
#		     ITBA 2011
#
#
.SILENT:

TARGET=tpeSO
OBJS= map.c backEnd.c company.c io.c
OBJS2= map.o backEnd.o company.o io.o
FIFO= ./transport/fifos/fifo.c
FIFO2= fifo.o
MSGQUEUE= ./transport/msgqueue/msgQueue.c
MSGQUEUE2= msgQueue.o
SOCKETS= ./transport/sockets/socket.c
SOCKETS2= socket.o
SHM= ./transport/sharedMemory/sharedMemory.c ./transport/sharedMemory/semaphore.c
SHM2= sharedMemory.o semaphore.o
VARRAY= ./transport/varray.c
VARRAY2= varray.o
MARSHALL = ./marshalling/marshalling.c
MARSHALL2 = marshalling.o
CC= gcc
COPTS= -Wall -ansi -pedantic -c -g -D_XOPEN_SOURCE=600
LDOPTS= -lpthread -o

backEnd.o: ./include/structs.h ./include/backEnd.h
company.o io.o map.o: ./include/structs.h ./include/backEnd.h ./include/api.h ./include/marshalling.h
varray.o: ../include/varray.h
fifo.o:
sockets.o:
msgqueue.o: ../../include/api.h
shareMemory.o: ../../include/api.h ../../include/semaphore.h ../../include/shm.h
semaphore.o: ../../include/semaphore.h
marshalling.o: ../include/api.h ../include/structs.h

shmem:
	echo Compiling $(OBJS) $(SHM) $(MARSHALL)
	$(CC) $(COPTS) $(OBJS) $(SHM) $(MARSHALL)
	echo Linking $(OBJS2) $(SHM2) $(MARSHALL2) to obtain $(TARGET)
	$(CC) $(LDOPTS) $(TARGET) $(OBJS2) $(SHM2) $(MARSHALL2)

msgqueue:
	echo Compiling $(OBJS) $(MSGQUEUE) $(MARSHALL)
	$(CC) $(COPTS) $(OBJS) $(MSGQUEUE) $(MARSHALL)
	echo Linking $(OBJS2) $(MSGQUEUE2) $(MARSHALL2) to obtain $(TARGET)
	$(CC) $(LDOPTS) $(TARGET) $(OBJS2) $(MSGQUEUE2) $(MARSHALL2)

fifo:
	echo Compiling $(OBJS) $(FIFO) &(VARRAY) $(MARSHALL)
	$(CC) $(COPTS) $(OBJS) $(FIFO) &(VARRAY) $(MARSHALL)
	echo Linking $(OBJS2) $(FIFO2) $(MARSHALL2) $(VARRAY2) to obtain $(TARGET)
	$(CC) $(LDOPTS) $(TARGET) $(OBJS2) $(FIFO2) $(VARRAY2) $(MARSHALL2)

sockets:
	echo Compiling $(OBJS) $(SOCKETS) $(MARSHALL)
	$(CC) $(COPTS) $(OBJS) $(SOCKETS) $(MARSHALL)
	echo Linking $(OBJS2) $(SOCKETS2) $(MARSHALL2) to obtain $(TARGET)
	$(CC) $(LDOPTS) $(TARGET) $(OBJS2) $(SOCKETS2) $(MARSHALL2)

clear:
	echo Clearing directory
	echo --- Removing *.o
	- rm *.o
	echo --- Removing $(TARGET)
	- rm $(TARGET)
