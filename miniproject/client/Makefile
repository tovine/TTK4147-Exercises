COMPILER=/home/student/TTK4147-Exercises/miniproject/avr32-linux-gcc

miniproject: miniproject.c miniproject.h
	$(COMPILER) -std=c99 miniproject.c -o miniproject -D_GNU_SOURCE -lpthread -lm

clean:
	rm miniproject
