#include <pthread.h>
//#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>

sem_t sem;

void* thread_work(int* args) {
	for (int i = 0; i < 10; i++) {
	sem_wait(&sem);
	printf("Thread id: %i took semaphore\n", (int) args[0]);
	sleep(1);
	sem_post(&sem);

	}
}

int main(void) {
	sem_init(&sem, 0, 3);

	pthread_t t_id[5];
	for (int i = 0; i < 5; i++) {
		pthread_create(&t_id[i], NULL, &thread_work, &t_id[i]);
	}
	for (int i = 0; i < 5; i++) {
		pthread_join(t_id[i], NULL);
	}

	return 0;
}
