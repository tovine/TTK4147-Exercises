#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>

pthread_mutex_t mutex;
bool running = true;

int var1 = 0, var2 = 0;

void* thread1_work(int* args) {
	while (running) {
		pthread_mutex_lock(&mutex);
		var1++;
		var2 = var1;
		pthread_mutex_unlock(&mutex);
	}
}

void* thread2_work(int* args) {
	for (int i = 0; i < 20; i++) {
		pthread_mutex_lock(&mutex);
		printf("Number 1 is %i, number 2 is %i\n", var1, var2);
		pthread_mutex_unlock(&mutex);
		usleep(100);
	}
	running = false;
}

int main(void) {
	pthread_mutex_init(&mutex, NULL);

	pthread_t t_id[2];
	pthread_create(&t_id[0], NULL, &thread1_work, NULL);
	pthread_create(&t_id[1], NULL, &thread2_work, NULL);
	for (int i = 0; i < 2; i++) {
		pthread_join(t_id[i], NULL);
	}

	return 0;
}
