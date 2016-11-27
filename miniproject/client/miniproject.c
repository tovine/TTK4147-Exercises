#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#include "miniproject.h"

#define timercmp(a,b,CMP) \
(((a)->tv_sec == (b)->tv_sec) ? \
((a)->tv_nsec CMP (b)->tv_nsec) : \
((a)->tv_sec CMP (b)->tv_sec))

#define UDP_RECV_BUF_LEN 255
// Configuration parameters
#define K_P             10.0
#define K_I             800.0
#define PERIOD_US       2000
#define PERIOD_S	0.002
#define SIMULATION_TIME 500000
#define REFERENCE       1.0

#define PART2		1

pthread_mutex_t sendLock;
sem_t receiveY;
sem_t receiveSignal;
volatile double g_y;
int g_running=1;
struct udp_conn udpSocket;

int main (){
	udp_init_client(&udpSocket, 9999, "192.168.0.1");

	pthread_mutex_init(&sendLock,NULL);
	sem_init(&receiveY,0,0);
	sem_init(&receiveSignal,0,0);

	pthread_t udpListener;
	pthread_create(&udpListener,NULL,&udpReceiver,NULL);

	pthread_t piThread;
	pthread_create(&piThread,NULL,&PIController,NULL);
#ifdef PART2
	pthread_t responderThread;
	pthread_create(&responderThread,NULL,&signalResponse,NULL);
#endif

	pthread_join(piThread, NULL);
	g_running = 0;
	udp_close(&udpSocket);
#ifdef PART2
	pthread_join(responderThread, NULL);
#endif
	// Wont actually exit unless the udp module is changed to accept timeout, or using the O_NONBLOCK flag, with some additional if test to see if we actually get data.
	pthread_join(udpListener, NULL);
	return 0;
}

int udp_init_client(struct udp_conn *udp, int port, char *ip)
{
	struct hostent *host;

	if ((host = gethostbyname(ip)) == NULL) return -1;

	udp->client_len = sizeof(udp->client);
	// define servers
	memset((char *)&(udp->server), 0, sizeof(udp->server));
	udp->server.sin_family = AF_INET;
	udp->server.sin_port = htons(port);
	bcopy((char *)host->h_addr, (char *)&(udp->server).sin_addr.s_addr, host->h_length);

	// open socket
	if ((udp->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) return udp->sock;

	return 0;
}

int udp_send(struct udp_conn *udp, char *buf, int len)
{
	return sendto(udp->sock, buf, len, 0, (struct sockaddr *)&(udp->server), sizeof(udp->server));
}

int udp_receive(struct udp_conn *udp, char *buf, int len)
{
	int res = recvfrom(udp->sock, buf, len, 0, (struct sockaddr *)&(udp->client), &(udp->client_len));

	return res;
}

void udp_close(struct udp_conn *udp)
{
	close(udp->sock);
	return;
}

int clock_nanosleep(struct timespec *next)
{
	struct timespec now;
	struct timespec sleep;

	// get current time
	clock_gettime(CLOCK_REALTIME, &now);

	// find the time the function should sleep
	sleep.tv_sec = next->tv_sec - now.tv_sec;
	sleep.tv_nsec = next->tv_nsec - now.tv_nsec;

	// if the nanosecon is below zero, decrement the seconds
	if (sleep.tv_nsec < 0)
	{
		sleep.tv_nsec += 1000000000;
		sleep.tv_sec -= 1;
	}

	// sleep
	nanosleep(&sleep, NULL);

	return 0;
}

void timespec_add_us(struct timespec *t, long us)
{
	// add microseconds to timespecs nanosecond counter
	t->tv_nsec += us*1000;

	// if wrapping nanosecond counter, increment second counter
	if (t->tv_nsec > 1000000000)
	{
		t->tv_nsec -= 1000000000;
		t->tv_sec += 1;
	}
}

/**
 * Thread that will do the regulation
 */
void * PIController(void * args){
	double error=0;
	double integral=0;
	double u=0;
	struct timespec now;
	struct timespec endSimulation;
	char dataBuffer[UDP_RECV_BUF_LEN] = {0};

	puts("Sent START");
	sendToServer("START");

	clock_gettime(CLOCK_REALTIME,&now);
	clock_gettime(CLOCK_REALTIME,&endSimulation);
	timespec_add_us(&endSimulation,SIMULATION_TIME);

	//Run until end of simulation
	while(timercmp(&endSimulation,&now,>) == 1){
		//Send get to server and w8 for reply
		waitForNewFeedback();

		error=REFERENCE-g_y;
		integral+=error*PERIOD_S;
		u=K_P*error+K_I*integral;

		//Send to network (blocked with mutex)
		sprintf(dataBuffer,"SET:%f",u);
		sendToServer(dataBuffer);

		//Update now time
		timespec_add_us(&now,PERIOD_US);
		//w8 for period
		clock_nanosleep(&now);
	}
	sendToServer("STOP");
	puts("Sent STOP");
	return args;
}

/**
 * Thread that receives data from the UDP socket
 */
void * udpReceiver(void * args){
	int status=0;
	while(g_running){
		char data[UDP_RECV_BUF_LEN];
		status=udp_receive(&udpSocket,data,UDP_RECV_BUF_LEN);
		if (status>0){
			if (strncmp(data,"GET_ACK",7)==0){
				g_y=atof(&data[8]);
				sem_post(&receiveY);
				puts(data);

			}
			else if (strncmp(data,"SIGNAL",6)==0){
				sem_post(&receiveSignal);
			}
		}
		
	}
	return args;
}

/**
 * Thread to respond to the reaction test
 */
void * signalResponse(void * args){
	char response[UDP_RECV_BUF_LEN]="SIGNAL_ACK";
	while (g_running){
		sem_wait(&receiveSignal);
		sendToServer(response);
	}
	return args;
}

/**
* Mutex protected udp send function
*/
void sendToServer(char * data){
	pthread_mutex_lock(&sendLock);
	udp_send(&udpSocket,data,strlen(data)+1);
	pthread_mutex_unlock(&sendLock);
}

/**
* Sends the get signal, and waits for the listener to free the semaphore.
*/
void waitForNewFeedback(void){
	sendToServer("GET");
	sem_wait(&receiveY);
	return;
}
