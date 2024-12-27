CC ?= gcc
CFLAGS ?=
LFLAGS ?=
BDIR ?= ./build/

all: $(BDIR) $(BDIR)ztk

$(BDIR)ztk: $(BDIR)ztk.o
	$(CC) -o $(BDIR)ztk $(LFLAGS) $(BDIR)ztk.o

$(BDIR)ztk.o: ./src/ztk.c
	$(CC) -o $(BDIR)ztk.o $(CFLAGS) -c ./src/ztk.c

$(BDIR):
	-mkdir $(BDIR)

run:
	$(BDIR)ztk
