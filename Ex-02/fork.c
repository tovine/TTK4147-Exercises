#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

extern int global_count;

int global_count = 0;

int main(void) {
	//pid_t pid;
	int local_count = 0;
	//fork();
	vfork();
	for (int i = 0; i < 1000; i++) {
		printf("PID: %d, local count: %i, global count: %i\n", getpid(), local_count++, global_count++);
		sleep(getpid() / 10000);
	}
	return 0;
}
