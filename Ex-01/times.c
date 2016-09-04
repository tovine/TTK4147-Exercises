#include <pthread.h>
#include <sys/times.h>
#include <stdio.h>

void busy_wait_delay(int seconds) {
	int i, dummy;
	//int tps = sysconf(_SC_CLK_TCK);
	int tps = sysconf(2);
	clock_t start;
	struct tms exec_time;
	
	times(&exec_time);
	start = exec_time.tms_utime;

	while( (exec_time.tms_utime - start) < (seconds * tps)) {
		for (i = 0; i < 1000; i++) {
			dummy = i;
		}
		times(&exec_time);
	}
}

void * t1_task(void * args) {
	printf("Thread one starting\n");
	sleep(5);
//	busy_wait_delay(4);
	printf("Thread one finished\n");
}

void * t2_task(void * args) {
	printf("Thread two starting\n");
	sleep(5);
//	busy_wait_delay(5);
	printf("Thread two finished\n");
}

int main(void) {
	int err;
	pthread_t thread1, thread2;

	err = pthread_create(&thread1, NULL, t1_task, NULL);
	err = pthread_create(&thread2, NULL, t2_task, NULL);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

}
