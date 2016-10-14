#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>
// Xenomai includes
#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
#include <rtdk.h>

#define MAIN_PRIO 1
#define TASK_PRIO 0

RT_SEM barrier;

void *task(void* bla){
	rt_printf("Task initializing\n");
	rt_printf("w8 for semaphore\n");
	rt_sem_p(&barrier,TM_INFINITE);//Block on barrier
	rt_printf("Task semaphore unblocked\n");
}

int main(void) {
	mlockall(MCL_CURRENT|MCL_FUTURE);
	rt_task_shadow(NULL,"Main task",MAIN_PRIO,T_CPU(1)); //Makes main a rt_task

	//Initializing semaphore
	rt_sem_create(&barrier,"Great Barrier",0,NULL);

	//Initialize rt_printf
	rt_print_auto_init(1);

	//We need 2 tasks that will w8 for semaphore.
	RT_TASK rt_tasks[2];

	//Initialize tasks
	rt_task_create(&rt_tasks[0],"Task1",0,TASK_PRIO,T_CPU(1) | T_JOINABLE);
	rt_task_create(&rt_tasks[1],"Task2",0,TASK_PRIO,T_CPU(1) | T_JOINABLE);

	//Start tasks
	rt_task_start(&rt_tasks[0],&task,NULL);
	rt_task_start(&rt_tasks[1],&task,NULL);

	//w8 for 100ms
	usleep(100000);

	//Broadcast so that semaphore unlocks
	rt_sem_broadcast(&barrier);

	//w8 for 100ms
	usleep(100000);

	//Wait for tasks to finish
	rt_task_join(&rt_tasks[0]);
	rt_task_join(&rt_tasks[1]);

	//Delete semaphore before exiting
	rt_sem_delete(&barrier);
}
