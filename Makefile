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
OBJS= frontEnd.o backEnd.o
FIFO= ./transport/fifos/fifo.o
FIFO2= fifo.o
VARRAY= ./transport/varray.o
VARRAY2= varray.o
MARSHALL = ./marshalling/marshalling.o
MARSHALL2 = marshalling.o
CC= gcc
COPTS= -Wall -ansi -pedantic -c -g 
LDOPTS= -lpthread -o 
OBJS2= map.o
OBJS3=io.o
OBJS4=company.o

$(TARGET):	 $(OBJS) $(OBJS2) $(OBJS3) $(OBJS4) $(FIFO) $(VARRAY) $(MARSHALL)
	echo Linking $(OBJS) $(FIFO2) $(VARRAY2) to obtain $(TARGET)
	$(CC) $(LDOPTS) $(TARGET) $(OBJS) $(FIFO2) $(VARRAY2) map.o io.o marshalling.o company.o

.c.o:
	echo Compiling $<
	$(CC) $(COPTS) $<

backEnd.o: ./include/structs.h ./include/backEnd.h
frontEnd.o: ./include/structs.h ./include/api.h
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

