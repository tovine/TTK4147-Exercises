iotest: io.o iotest.c
	gcc -std=c99 iotest.c io.o -o iotest -D_GNU_SOURCE -lcomedi -lrt -lpthread -lm

io.o: io.h io.c
	gcc -c io.c -o io.o -lcomedi -lrt -pthread -lm

clean:
	rm *.o iotest
