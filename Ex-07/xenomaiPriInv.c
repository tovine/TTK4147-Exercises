#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>
// Xenomai includes
#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
#include <rtdk.h>

#define MAIN_PRIO 4

RT_SEM barrier;
RT_SEM resource;
int timeUnit=10;

// PROVIDED IN EXERCISE //
void print_pri(RT_TASK *task, char *s){
	struct rt_task_info temp;
	rt_task_inquire(task, &temp);
	rt_printf("b:%i c:%i ", temp.bprio, temp.cprio);
	rt_printf(s);
}

int rt_task_sleep_ms(unsigned long delay){
	return rt_task_sleep(1000*1000*delay);
}

void busy_wait_ms(unsigned long delay){
	unsigned long count = 0;
	while (count <= delay*10){
		rt_timer_spin(1000*100);
		count++;
	}
}

//TASKS WITH LOW,MEDIUM AND HIGH PRIORITY //
void* l(void*bla){
	rt_sem_p(&barrier,TM_INFINITE);//Block on barrier
	rt_sem_p(&resource,TM_INFINITE);
	rt_printf("LOW: Acquired Resource\n");
	busy_wait_ms(3*timeUnit);
	rt_printf("LOW: Finished busy_wait for 3 timeUnits\n");
	rt_sem_v(&resource);
	rt_printf("LOW: Released Resource\n");
}

void* m(void*bla){
	rt_sem_p(&barrier,TM_INFINITE);//Block on barrier
	//rt_sem_p(&resource,TM_INFINITE);
	rt_task_sleep_ms(1*timeUnit);
	rt_printf("MEDIUM: Finished sleep for 1 timeUnit\n");
	busy_wait_ms(5*timeUnit);
	rt_printf("MEDIUM: Finished busy_wait for 5 timeUnits\n");
	//rt_sem_v(&resource);	
}

void* h(void*bla){
	rt_sem_p(&barrier,TM_INFINITE);//Block on barrier
	rt_task_sleep_ms(2*timeUnit);
	rt_printf("HIGH: Finished sleep for 2 timeUnit\n");
	rt_sem_p(&resource,TM_INFINITE);
	rt_printf("HIGH: Acquired Resource\n");
	busy_wait_ms(2*timeUnit);
	rt_printf("HIGH: Finished busy_wait for 2 timeUnits\n");
	rt_sem_v(&resource);	
	rt_printf("HIGH: Released Resource\n");
}

int main(void) {
	mlockall(MCL_CURRENT|MCL_FUTURE);
	rt_task_shadow(NULL,"Main task",MAIN_PRIO,T_CPU(1)); //Makes main a rt_task

	//Initializing semaphore
	rt_sem_create(&barrier,"Great Barrier",0,NULL);
	rt_sem_create(&resource,"Resource",1,NULL);

	//Initialize rt_printf
	rt_print_auto_init(1);

	//We need 3 tasks that will w8 for semaphore.
	RT_TASK rt_tasks[3];

	//Initialize tasks
	rt_task_create(&rt_tasks[0],"Task1",0,1,T_CPU(1) | T_JOINABLE);
	rt_task_create(&rt_tasks[1],"Task2",0,2,T_CPU(1) | T_JOINABLE);
	rt_task_create(&rt_tasks[2],"Task3",0,3,T_CPU(1) | T_JOINABLE);

	//Start tasks
	rt_task_start(&rt_tasks[0],&l,NULL);
	rt_task_start(&rt_tasks[1],&m,NULL);
	rt_task_start(&rt_tasks[2],&h,NULL);

	//w8 for 100ms
	usleep(100000);

	//Broadcast so that semaphore unlocks
	rt_sem_broadcast(&barrier);

	//w8 for 100ms
	usleep(100000);

	//Wait for tasks to finish
	rt_task_join(&rt_tasks[0]);
	rt_task_join(&rt_tasks[1]);
	rt_task_join(&rt_tasks[2]);

	//Delete semaphore before exiting
	rt_sem_delete(&barrier);
}
