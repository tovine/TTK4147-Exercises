list_test : linked_list.o list_test.c
	gcc -std=c99 list_test.c linked_list.o

linked_list.o : linked_list.h linked_list.c
	gcc -std=c99 -c linked_list.c -o linked_list.o

clean:
	rm *.o
