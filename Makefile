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
CC= gcc
COPTS= -Wall -ansi -pedantic -c -g
LDOPTS= -o
TARGET2=map
OBJS2= map.o
TARGET3=io
OBJS3=io.o
TARGET4=company
OBJS4=company.o

$(TARGET):	 $(OBJS) $(OBJS2) $(OBJS3) $(OBJS4)
	echo Linking $(OBJS) to obtain $(TARGET)
	$(CC) $(LDOPTS) $(TARGET) $(OBJS)
	$(CC) $(LDOPTS) $(TARGET2) $(OBJS2)
	$(CC) $(LDOPTS) $(TARGET3) $(OBJS3)
	$(CC) $(LDOPTS) $(TARGET4) $(OBJS4)

.c.o:
	echo Compiling $<
	$(CC) $(COPTS) $<

backEnd.o: structs.h backEnd.h
frontEnd.o: structs.h

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

