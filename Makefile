#
#
#		Makefile for tpeSO
#				Jose Ignacio Galindo
#				Federico Homovc
#				Nicolas Loreti
#				    ITBA 2011
#
#
.SILENT:

TARGET=tpeSO
OBJS= map.o backEnd.o company.o io.o
FIFO= ./transport/fifos/fifo.o
FIFO2= fifo.o
MSGQUEUE= ./transport/msgqueue/msgQueue.o
MSGQUEUE2= msgQueue.o
SOCKETS= ./transport/sockets/socket.o
SOCKETS2= socket.o
VARRAY= ./transport/varray.o
VARRAY2= varray.o
MARSHALL = ./marshalling/marshalling.o
MARSHALL2 = marshalling.o
CC= gcc
COPTS= -Wall -ansi -pedantic -c -g -D_XOPEN_SOURCE=600
LDOPTS= -lpthread -o 

$(TARGET):	 $(OBJS) $(MSGQUEUE) $(VARRAY) $(MARSHALL)
	echo Linking $(OBJS) $(MSGQUEUE2) $(VARRAY2) to obtain $(TARGET)
	$(CC) $(LDOPTS) $(TARGET) $(OBJS) $(MSGQUEUE2) $(VARRAY2) $(MARSHALL2)

.c.o:
	echo Compiling $<
	$(CC) $(COPTS) $<

backEnd.o: ./include/structs.h ./include/backEnd.h
fifo.o: ../../include/varray.h
varray.o: ../include/varray.h
io.o: ./include/structs.h ./include/backEnd.h ./include/api.h ./include/varray.h
map.o: ./include/structs.h ./include/backEnd.h ./include/api.h ./include/varray.h

clear:
	echo Clearing directory
	echo --- Removing *.o
	- rm *.o
	echo --- Removing $(TARGET)
	- rm $(TARGET)

