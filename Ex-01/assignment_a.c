#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

void allocate(int value) {
	int *ptr = NULL;
	ptr = malloc(100000 * sizeof(int));
	if (ptr == NULL) {
		perror("Failed to allocate memory: ");
		exit(errno);
	}
	*ptr = value;
	printf("test of allocated memory: %i\n", ptr);
}

int main(char * args) {
	while(1)
	allocate(1337);
}
