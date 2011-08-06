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

$(TARGET):	 $(OBJS)
	echo Linking $(OBJS) to obtain $(TARGET)
	$(CC) $(LDOPTS) $(TARGET) $(OBJS)

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

