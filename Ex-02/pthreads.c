#include <pthread.h>
#include <stdio.h>

extern int global_count;
int global_count;

void* thread_work(void* args) {
	int local_count = 0;
	for (int i = 0; i < 500; i++) {
		printf("local count: %i, global count: %i\n", local_count++, global_count++);
	}
}

int main(void) {
	global_count = 0;
	pthread_t t_id[2];
	pthread_create(&t_id[0], NULL, &thread_work, NULL);
	pthread_create(&t_id[1], NULL, &thread_work, NULL);
	pthread_join(t_id[1], NULL);
	pthread_join(t_id[0], NULL);

	return 0;
}
