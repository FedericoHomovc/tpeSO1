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
OBJS= ./application/map.c ./utils/backEnd.c ./application/company.c ./application/io.c
OBJS2= map.o backEnd.o company.o io.o
FIFO= ./transport/fifos/fifo.c
FIFO2= fifo.o
MSGQUEUE= ./transport/msgqueue/msgQueue.c
MSGQUEUE2= msgQueue.o
SOCKETS= ./transport/sockets/socket.c ./utils/semaphore.c
SOCKETS2= socket.o semaphore.o
SHM= ./transport/sharedMemory/sharedMemory.c ./utils/semaphore.c
SHM2= sharedMemory.o semaphore.o
MARSHALL = ./marshalling/marshalling.c
MARSHALL2 = marshalling.o
CC= gcc
COPTS= -Wall -ansi -pedantic -c -g -D_XOPEN_SOURCE=600
LDOPTS= -lpthread -o

backEnd.o: ./include/structs.h ./include/backEnd.h
company.o io.o map.o: ./include/structs.h ./include/backEnd.h ./include/api.h ./include/marshalling.h
fifo.o: ../../include/api.h ../../include/backEnd.h
sockets.o: ../../include/api.h ../../include/semaphore.h
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
	echo Compiling $(OBJS) $(FIFO)  $(MARSHALL)
	$(CC) $(COPTS) $(OBJS) $(FIFO)  $(MARSHALL)
	echo Linking $(OBJS2) $(FIFO2) $(MARSHALL2)  to obtain $(TARGET)
	$(CC) $(LDOPTS) $(TARGET) $(OBJS2) $(FIFO2)  $(MARSHALL2)

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
