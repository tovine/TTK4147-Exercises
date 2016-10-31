#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/dispatch.h>
#include <sys/mman.h>
#include <pthread.h>

#define BUFFER_MAX_SIZE 128
#define NUM_CLIENTS 4

int createSharedMemory(){
	return shm_open("/sharepid", O_RDWR ,S_IRWXU);
}

struct pid_data{
	pthread_mutex_t pid_mutex;
	pid_t pid;
};

int getServerPID(void) {
	int shm=createSharedMemory();
	//ftruncate(shm,sizeof(pid_data));
	void *mapped = mmap(0,sizeof(struct pid_data),PROT_READ | PROT_WRITE,MAP_SHARED,shm,0);
	struct pid_data *ptr = (struct pid_data*) mapped;
	//pthread_mutexattr_t myattr;

    /*pthread_mutexattr_init(&myattr);
    pthread_mutexattr_setpshared(&myattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&ptr->pid_mutex, &myattr );
    */

	printf("PID: %d, mutex: %d\n",(int)ptr->pid, ptr->pid_mutex.__owner);
	return ptr->pid;
}

int startClient(int serverPID) {
	return ConnectAttach(0, serverPID, 1, 0, 0);
}

void stopClient(int channelId) {
	ConnectDetach(channelId);
}

int sendMsg(int channelId, char * message) {
	char return_buf[3];
	return_buf[2] = '\0';
	int size = 0;
	while(message[size++] != '\0' && size < BUFFER_MAX_SIZE) {}; // get the string length, stop at the max
	int retval = MsgSend(channelId, message, size, &return_buf, 2);
	printf("Server responded: %s (my priority: %d)\n", &return_buf, get_priority());
	return retval;
}

void clientWorker(int* arg) {
	int priority = (int) arg[0];
	set_priority(priority);
	int channelId = startClient(getServerPID());
	printf("Client with priority %d started\n", get_priority());
	sendMsg(channelId, "Hello, this is client!");
	stopClient(channelId);
}

int set_priority(int priority) {
     int policy;
     struct sched_param param;
     // check priority in range
     if (priority < 1 || priority > 63) return -1;
     // set priority
     pthread_getschedparam(pthread_self(), &policy, &param);
     param.sched_priority = priority;
     return pthread_setschedparam(pthread_self(), policy, &param);
}

int get_priority() {
     int policy;
     struct sched_param param;
     // get priority
     pthread_getschedparam(pthread_self(), &policy, &param);
     return param.sched_curpriority;
}

int main(int argc, char *argv[]) {
	printf("Hello client\n");
	set_priority(10);
	pthread_t threads[NUM_CLIENTS];
	int i, priority[NUM_CLIENTS];
	for (i = 0; i < NUM_CLIENTS; i++) {
		priority[i] = 2*i+1;
		pthread_create(&threads[i], NULL, &clientWorker, &priority[i]);
	}
	for (i = 0; i < NUM_CLIENTS; i++)
		pthread_join(threads[i], NULL);

	return EXIT_SUCCESS;
}
