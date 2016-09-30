#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>

#include "io.h"

#define CHAN_A 1
#define CHAN_B 2
#define CHAN_C 3

bool running = true;

int set_cpu(int cpu_number) {
	// setting cpu set to the selected cpu
	cpu_set_t cpu;
	CPU_ZERO(&cpu);
	CPU_SET(cpu_number, &cpu);
	// set cpu set to current thread and return
	return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t),&cpu);
}

/** Function to monitor the inputs and respond */
void* monitor_input(int* args) {
	set_cpu(1);
	int channel = args[0];
	printf("Worker for input %d ready and listening\n", channel);
	while(running) {
		// Poll the input and respond
		if(!io_read(channel)) {
			io_write(channel, 0);
			usleep(5);
			io_write(channel, 1);
		}
		usleep(10);
	}
}

int main(void) {
	if(io_init() == -1) {
		puts("Something went wrong trying to init the I/O card\n");
		return 1;
	}
	printf("I/O card initialized, current values: %d, %d, %d\n", io_read(CHAN_A), io_read(CHAN_B), io_read(CHAN_C));
	for (int i = 1; i <= 3; i++) {
		sleep(1);
		io_write(i, !io_read(i));
		printf("Changed value of channel %d to %d\n", i, io_read(i));
	}
	
	pthread_t t_id[3];
	const int A = CHAN_A, B = CHAN_B, C = CHAN_C;
	pthread_create(&t_id[0], NULL, &monitor_input, &A);
	pthread_create(&t_id[1], NULL, &monitor_input, &B);
	pthread_create(&t_id[2], NULL, &monitor_input, &C);
	pthread_join(t_id[0], NULL);
	pthread_join(t_id[2], NULL);
	pthread_join(t_id[3], NULL);
}
