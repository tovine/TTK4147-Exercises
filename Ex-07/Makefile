XENO_DESTDIR:=
XENO_CONFIG:=$(XENO_DESTDIR)/usr/bin/xeno-config
XENO_CFLAGS:=$(shell DESTDIR=$(XENO_DESTDIR) $(XENO_CONFIG) --skin native --cflags)
XENO_LIBS:=$(shell DESTDIR=$(XENO_DESTDIR) $(XENO_CONFIG) --skin native --ldflags)

xenomaiSem: xenomaiSem.c
	gcc -std=gnu99 xenomaiSem.c -o xenomaiSem $(XENO_CFLAGS) $(XENO_LIBS) -lm 
xenomaiPriInv: xenomaiPriInv.c
	gcc -std=gnu99 xenomaiPriInv.c -o xenomaiPriInv $(XENO_CFLAGS) $(XENO_LIBS) -lm 
xenomaiMutex: xenomaiMutex.c
	gcc -std=gnu99 xenomaiMutex.c -o xenomaiMutex $(XENO_CFLAGS) $(XENO_LIBS) -lm 
xenomaiICPP: xenomaiICPP.c
	gcc -std=gnu99 xenomaiICPP.c -o xenomaiICPP $(XENO_CFLAGS) $(XENO_LIBS) -lm 

clean:
	rm *.o xenomaiSem xenomaiPriInv xenomaiMutex xenomaiICPP
