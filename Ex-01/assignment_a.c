#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void allocate(int value) {
	int *ptr = NULL;
	ptr = malloc(100000 * sizeof(int));
	*ptr = value;
	printf("test of allocated memory: %i\n", ptr);
}

int main(char * args) {
	while(1)
	allocate(1337);
}
