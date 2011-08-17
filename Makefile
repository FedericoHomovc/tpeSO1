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
CC= gcc
COPTS= -Wall -ansi -pedantic -c -g 
LDOPTS= -lpthread -o 
TARGET2=map
OBJS2= map.o
TARGET3=io
OBJS3=io.o
TARGET4=company
OBJS4=company.o

$(TARGET):	 $(OBJS) $(OBJS2) $(OBJS3) $(OBJS4) $(FIFO) $(VARRAY)
	echo Linking $(OBJS) $(FIFO2) $(VARRAY2) to obtain $(TARGET)
	$(CC) $(LDOPTS) $(TARGET) $(OBJS) $(FIFO2) $(VARRAY2)
	$(CC) $(LDOPTS) $(TARGET2) $(OBJS2) $(FIFO2) $(VARRAY2) backEnd.o
	$(CC) $(LDOPTS) $(TARGET3) $(OBJS3) backEnd.o
	$(CC) $(LDOPTS) $(TARGET4) $(OBJS4) backEnd.o

.c.o:
	echo Compiling $<
	$(CC) $(COPTS) $<

backEnd.o: structs.h backEnd.h
frontEnd.o: structs.h ./include/api.h
fifo.o: ../../include/varray.h
varray.o: ../include/varray.h
io.o: ./include/structs.h ./include/backEnd.h ./include/io.h ./include/api.h ./include/varray.h
map.o: ./include/structs.h ./include/backEnd.h ./include/io.h ./include/api.h ./include/varray.h


clear:
	echo Clearing directory
	echo --- Removing *.o
	- rm *.o
	echo --- Removing $(TARGET)
	- rm $(TARGET)
	echo --- Removing $(TARGET2)
	- rm $(TARGET2)
	echo --- Removing $(TARGET3)
	- rm $(TARGET3)
	echo --- Removing $(TARGET4)
	- rm $(TARGET4)

