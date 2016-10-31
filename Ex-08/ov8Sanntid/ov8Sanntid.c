#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/dispatch.h>
#include <sys/mman.h>
#include <pthread.h>

#define BUFFER_SIZE 128

int createSharedMemory(){
	return shm_open("/sharepid", O_RDWR | O_CREAT,S_IRWXU);
}

struct pid_data{
	pthread_mutex_t pid_mutex;
	pid_t pid;
};

void assignment_a(void) {
    int shm = createSharedMemory();
    ftruncate(shm, sizeof (struct pid_data));
    void *mapped = mmap(0,sizeof(struct pid_data),PROT_READ | PROT_WRITE,MAP_SHARED,shm,0);
    struct pid_data *ptr = (struct pid_data*)mapped;
    pthread_mutexattr_t myattr;
    pthread_mutexattr_init(&myattr);
    pthread_mutexattr_setpshared(&myattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&ptr->pid_mutex, &myattr);
    ptr->pid = getpid();
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

void startServer(void) {
	int channel_id = ChannelCreate(_NTO_CHF_FIXED_PRIORITY); // _NTO_CHF_FIXED_PRIORITY
	char recvBuf[BUFFER_SIZE + 1];
	recvBuf[BUFFER_SIZE] = '\0';
	set_priority(4);
	while(1) {
		struct _msg_info msgInfo;
		int msg_id = MsgReceive(channel_id, &recvBuf, BUFFER_SIZE, &msgInfo);
		printf("Current priority (before): %d\n", get_priority());
		printf("Got message from client pid %d, thread id %d: %s\n", msgInfo.pid, msgInfo.tid, &recvBuf);
		printf("Current priority (after):%d\n", get_priority());
		MsgReply(msg_id, EOK, "OK", 2);
	}
}

int main(int argc, char *argv[]) {
	printf("Hello Server\n");
    assignment_a();
	startServer();
	return EXIT_SUCCESS;
}
