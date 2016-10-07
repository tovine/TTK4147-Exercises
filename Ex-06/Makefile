XENO_DESTDIR:=
XENO_CONFIG:=$(XENO_DESTDIR)/usr/bin/xeno-config
XENO_CFLAGS:=$(shell DESTDIR=$(XENO_DESTDIR) $(XENO_CONFIG) --skin native --cflags)
XENO_LIBS:=$(shell DESTDIR=$(XENO_DESTDIR) $(XENO_CONFIG) --skin native --ldflags)

iotest: io.o iotest.c
	gcc -std=gnu99 iotest.c io.o -o iotest $(XENO_CFLAGS) -lcomedi $(XENO_LIBS) -lm 

io.o: io.h io.c
	gcc -c io.c -o io.o -lcomedi -lrt -pthread -lm

clean:
	rm *.o iotest
