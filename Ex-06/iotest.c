#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>

#include <sys/mman.h>
// Xenomai includes
#include <native/task.h>
#include <native/timer.h>

#include "io.h"

#define CHAN_A 1
#define CHAN_B 2
#define CHAN_C 3

#define NUM_DISTURBANCE_THREADS 10
#define TEST_PERIOD_US 10
#define TEST_PERIOD_NS (TEST_PERIOD_US * 1000)

#define RESPONSE_TASK_PRIO 10

bool running = true;

int set_cpu(int cpu_number) {
	// setting cpu set to the selected cpu
	cpu_set_t cpu;
	CPU_ZERO(&cpu);
	CPU_SET(cpu_number, &cpu);
	// set cpu set to current thread and return
	return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t),&cpu);
}

void timespec_add_us(struct timespec *t, long us) {
	t->tv_nsec += us*1000;
	if (t->tv_nsec > 1000000000) {
		t->tv_nsec = t->tv_nsec - 1000000000;
		t->tv_sec += 1;
	}
}

/** Function to monitor the inputs and respond */
void* monitor_input(int* args) {
//	set_cpu(1);
	int channel = args[0];
//	struct timespec next;
//	clock_gettime(CLOCK_REALTIME, &next);
	printf("Worker for input %d ready and listening\n", channel);
	while(running) {
		// Poll the input and respond
		if(!io_read(channel)) {
			io_write(channel, 0);
			usleep(5);
			io_write(channel, 1);
		}
		rt_task_wait_period(NULL);
//		pthread_yield();
//		usleep(10);
//		timespec_add_us(&next, TEST_PERIOD_US);
//		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next, NULL);
	}
}

/** 
* Function to waste cycles and disturb the main task - NOTE: do NOT run as rt_task!!
*/
void* disturbance_task(int* args) {
	set_cpu(1);
	int channel = args[0];
	printf("Disturbance thread %d running\n", channel);
	while(running) {
		1 + 1;
	}
}

/** Function to say hello every 500ms */
void* nag_task(int* args) {
//	struct timespec next;
//	clock_gettime(CLOCK_REALTIME, &next);
	while (running) {
//		timespec_add_us(&next, 500000);
		puts("Hello");
//		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next, NULL);
		rt_task_wait_period(NULL);
	}
}

int main(void) {
	mlockall(MCL_CURRENT|MCL_FUTURE);
	if(io_init() == -1) {
		puts("Something went wrong trying to init the I/O card\n");
		return 1;
	}
	printf("I/O card initialized, current values: %d, %d, %d\n", io_read(CHAN_A), io_read(CHAN_B), io_read(CHAN_C));
	for (int i = 1; i <= 3; i++) {
		io_write(i,1); 
		printf("Changed value of channel %d to 1\n", i);
	}

	RT_TASK rt_tasks[4];
	pthread_t t_id[14];
	const int A = CHAN_A, B = CHAN_B, C = CHAN_C;
/*	pthread_create(&t_id[0], NULL, &monitor_input, &A);
	pthread_create(&t_id[1], NULL, &monitor_input, &B);
	pthread_create(&t_id[2], NULL, &monitor_input, &C);
*/
	rt_task_create(&rt_tasks[0], "Input monitor A", 0, RESPONSE_TASK_PRIO, T_CPU(1) | T_JOINABLE);
	rt_task_set_periodic(&rt_tasks[0], TM_NOW, TEST_PERIOD_NS);
	rt_task_start(&rt_tasks[0], &monitor_input, &A);
	rt_task_create(&rt_tasks[1], "Input monitor B", 0, RESPONSE_TASK_PRIO, T_CPU(1) | T_JOINABLE);
	rt_task_set_periodic(&rt_tasks[1], TM_NOW, TEST_PERIOD_NS);
	rt_task_start(&rt_tasks[1], &monitor_input, &B);
	rt_task_create(&rt_tasks[2], "Input monitor C", 0, RESPONSE_TASK_PRIO, T_CPU(1) | T_JOINABLE);
	rt_task_set_periodic(&rt_tasks[2], TM_NOW, TEST_PERIOD_NS);
	rt_task_start(&rt_tasks[2], &monitor_input, &C);

	for (int i = 0; i < NUM_DISTURBANCE_THREADS; i++) {
		pthread_create(&t_id[i+3], NULL, &disturbance_task, &i);
		usleep(5000);
	}
	rt_task_create(&rt_tasks[3], "Hello task", 0, 0, T_CPU(0) | T_JOINABLE);
	rt_task_set_periodic(&rt_tasks[3], TM_NOW, 500000000);
	rt_task_start(&rt_tasks[3], &nag_task, NULL);
//	pthread_create(&t_id[13], NULL, &nag_task, NULL);
/*	pthread_join(&t_id[0], NULL);
	pthread_join(&t_id[2], NULL);
	pthread_join(&t_id[3], NULL);
*/
	rt_task_join(&rt_tasks[0]);
	rt_task_join(&rt_tasks[1]);
	rt_task_join(&rt_tasks[2]);
	for (int i = 0; i < 10; i++) {
		pthread_join(&t_id[i+3], NULL);
	}
	pthread_join(&t_id[13], NULL);
}
