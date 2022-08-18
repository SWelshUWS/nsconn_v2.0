CC = gcc
INCLUDE = ../include
ODIR = obj
AM_CFLAGS = -I.

# dependencies
_DEPS = nsconn.h
DEPS = $(patsubstr%,$(INCLUDE)/%,$(_DEPS))

# objects for compilation
_OBJ = nsconn.o btsock.o outsock.o parsens.o format.o forward.o
OBJ = $(patsubst%,$(ODIR)/%,$(_OBJ))


%.o: %.c $(DEPS)
    $(CC) -c -o $@ $< $(AM_CFLAGS)

# compile objects into executable
nsconn: $(OBJ)
    $(CC) -o $@ $^ $(AM_CFLAGS)




