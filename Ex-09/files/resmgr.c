#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include "fifo.h"

dispatch_t              *dpp;
resmgr_attr_t           resmgr_attr;
dispatch_context_t      *ctp;
resmgr_connect_funcs_t  connect_funcs;
resmgr_io_funcs_t       io_funcs;
iofunc_attr_t           io_attr;

#define BUFFER_MAX_SIZE WIDTH

int io_read_str(resmgr_context_t *ctp, io_read_t *msg, iofunc_ocb_t *ocb);
int io_write_str(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb);
int io_read_count(resmgr_context_t *ctp, io_read_t *msg, iofunc_ocb_t *ocb);
int io_write_count(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb);
int io_read_fifo(resmgr_context_t *ctp, io_read_t *msg, iofunc_ocb_t *ocb);
int io_write_fifo(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb);
void respond_to_blocked_client(resmgr_context_t *ctp, int client_id);

char buf[BUFFER_MAX_SIZE+1] = "Hello World\n";
int count = 0, dir = 0; // Dir: -1 down 0 stop, 1 up
pthread_mutex_t mutex;

fifo_t queue;

void worker_thread(void * args) {
	while(1) {
		pthread_mutex_lock(&mutex);
		count = count + dir;
		pthread_mutex_unlock(&mutex);
		delay(100);
	}
}

void error(char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	printf("Start resource manager\n");

	// create dispatch.
	if (!(dpp = dispatch_create()))
		error("Create dispatch");

	// initialize resource manager attributes.
	memset(&resmgr_attr, 0, sizeof(resmgr_attr));
	resmgr_attr.nparts_max = 1;
	resmgr_attr.msg_max_size = 2048;

	// set standard connect and io functions.
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS, &io_funcs);
	iofunc_attr_init(&io_attr, S_IFNAM | 0666, 0, 0);

	// override functions
	// Oppgave 1
//	io_funcs.read = io_read_str;
//	io_funcs.write = io_write_str;
	// Oppgave 2
	io_funcs.read = io_read_count;
	io_funcs.write = io_write_count;
	pthread_t worker;
	pthread_create(&worker, NULL, &worker_thread, NULL);
	// Oppgave 3
//	io_funcs.read = io_read_fifo;
//	io_funcs.write = io_write_fifo;
//	init_fifo(&queue);


	// establish resource manager
	if (resmgr_attach(dpp, &resmgr_attr, "/dev/myresource", _FTYPE_ANY, 0, &connect_funcs, &io_funcs, &io_attr) < 0)
		error("Resmanager attach");
	ctp = dispatch_context_alloc(dpp);

	// wait forever, handling messages.
	while(1)
	{
		if (!(ctp = dispatch_block(ctp)))
			error("Dispatch block");
		dispatch_handler(ctp);
	}
	// Oppgave 2
//	pthread_join(&worker, NULL);
	exit(EXIT_SUCCESS);
}

int io_read_str(resmgr_context_t *ctp, io_read_t *msg, iofunc_ocb_t *ocb)
{

	if(!ocb->offset)
	{
		pthread_mutex_lock(&mutex);
		// set data to return
		SETIOV(ctp->iov, buf, strlen(buf));
		_IO_SET_READ_NBYTES(ctp, strlen(buf));
		pthread_mutex_unlock(&mutex);

		// increase the offset (new reads will not get the same data)
		ocb->offset = 1;

		// return
		return (_RESMGR_NPARTS(1));
	}
	else
	{
		// set to return no data
		_IO_SET_READ_NBYTES(ctp, 0);

		// return
		return (_RESMGR_NPARTS(0));
	}
}

int io_write_str(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb)
{

	if(!ocb->offset)
	{
		// set data to return
//		SETIOV(ctp->iov, buf, strlen(buf));
		int size = BUFFER_MAX_SIZE;
		if (msg->i.nbytes < BUFFER_MAX_SIZE)
			size = msg->i.nbytes;
		pthread_mutex_lock(&mutex);
		resmgr_msgread(ctp, buf, size, sizeof(msg->i));
		buf[size] = '\0';
		_IO_SET_WRITE_NBYTES(ctp, strlen(buf));
		pthread_mutex_lock(&mutex);

		// increase the offset (new reads will not get the same data)
		ocb->offset = 1;
		printf("New contents: %s\n", buf);
		// return
		return (_RESMGR_NPARTS(0));
	}
	else
	{
		// set to return no data
		_IO_SET_WRITE_NBYTES(ctp, 0);

		// return
		return (_RESMGR_NPARTS(0));
	}
}

int io_read_count(resmgr_context_t *ctp, io_read_t *msg, iofunc_ocb_t *ocb)
{

	if(!ocb->offset)
	{
		pthread_mutex_lock(&mutex);
		// set data to return
		sprintf(&buf, "%d\n", count);
		SETIOV(ctp->iov, buf, strlen(buf));
		_IO_SET_READ_NBYTES(ctp, strlen(buf));
		pthread_mutex_unlock(&mutex);

		// increase the offset (new reads will not get the same data)
		ocb->offset = 1;

		// return
		return (_RESMGR_NPARTS(1));
	}
	else
	{
		// set to return no data
		_IO_SET_READ_NBYTES(ctp, 0);

		// return
		return (_RESMGR_NPARTS(0));
	}
}

int io_write_count(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb)
{

	if(!ocb->offset)
	{
		// set data to return
//		SETIOV(ctp->iov, buf, strlen(buf));
		int size = BUFFER_MAX_SIZE;
		if (msg->i.nbytes < BUFFER_MAX_SIZE)
			size = msg->i.nbytes;
		pthread_mutex_lock(&mutex);
		resmgr_msgread(ctp, buf, size, sizeof(msg->i));
		buf[size] = '\0';
		_IO_SET_WRITE_NBYTES(ctp, strlen(buf));

		// increase the offset (new reads will not get the same data)
		ocb->offset = 1;
		printf("New contents: %s\n", buf);
		if (strncmp(buf, "up", 2) == 0) dir = 1;
		else if (strncmp(buf, "down", 4) == 0) dir = -1;
		else if (strncmp(buf, "stop", 4) == 0) dir = 0;
		pthread_mutex_unlock(&mutex);
		// return
		return (_RESMGR_NPARTS(0));
	}
	else
	{
		// set to return no data
		_IO_SET_WRITE_NBYTES(ctp, 0);

		// return
		return (_RESMGR_NPARTS(0));
	}
}

int io_read_fifo(resmgr_context_t *ctp, io_read_t *msg, iofunc_ocb_t *ocb)
{
	if(!ocb->offset)
	{
		int nonblock, status;
		if ((status = iofunc_read_verify (ctp, msg, ocb, &nonblock)) != EOK)
			return (status);

		printf("Client incoming, rcvid: %d\n", ctp->rcvid);

		pthread_mutex_lock(&queue.resource_mutex);
		puts("Blocked clients:");
		fifo_print_blocked_ids(&queue);
		puts("Items in queue:");
		fifo_print(&queue);

		if (!fifo_status(&queue)) {
			// Queue is empty, return
			pthread_mutex_unlock(&queue.resource_mutex);
			if (nonblock) {
				// set to return no data
				_IO_SET_READ_NBYTES(ctp, 0);
				return (_RESMGR_NPARTS(0));
			} else {
				fifo_add_blocked_id(&queue, ctp->rcvid);
				return (_RESMGR_NOREPLY);
			}
		} //else {
			//fifo_add_blocked_id(&queue, ctp->rcvid);
/*			pthread_mutex_unlock(&queue.resource_mutex);
			int fifostatus = 0;
			while(!fifostatus) {
				// Block until there's a message available
				pthread_mutex_lock(&queue.resource_mutex);
				fifostatus = fifo_status(&queue);
				if (fifostatus && queue.blocked_id[queue.blockedHead] == ctp->rcvid) {
					// We're next - remove from blocked list and return data
					fifo_rem_blocked_id(&queue);
					printf("Client unblocked, rcvid: %d\n", ctp->rcvid);
					break;
				}
				pthread_mutex_unlock(&queue.resource_mutex);
				sleep(1);
			}
*/
	//	}

		fifo_rem_string(&queue, buf);
		puts("Blocked clients:");
		fifo_print_blocked_ids(&queue);
		puts("Items in queue:");
		fifo_print(&queue);

		// set data to return
		SETIOV(ctp->iov, buf, strlen(buf));
		_IO_SET_READ_NBYTES(ctp, strlen(buf));
		pthread_mutex_unlock(&queue.resource_mutex);

		printf("Client served, rcvid: %d\n", ctp->rcvid);

		// increase the offset (new reads will not get the same data)
		ocb->offset = 1;

		// return
		return (_RESMGR_NPARTS(1));
	}
	else
	{
		// set to return no data
		_IO_SET_READ_NBYTES(ctp, 0);

		// return
		return (_RESMGR_NPARTS(0));
	}
}

int io_write_fifo(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb)
{

	if(!ocb->offset)
	{
		// set data to return
		int size = WIDTH; // defined in fifo.h
		if (msg->i.nbytes < WIDTH)
			size = msg->i.nbytes;
		pthread_mutex_lock(&queue.resource_mutex);
		resmgr_msgread(ctp, buf, size, sizeof(msg->i));
		buf[size] = '\0';
		if (fifo_add_string(&queue, buf)) { // If queue is full, message gets discarded
			//_IO_SET_WRITE_NBYTES(ctp, strlen(buf));
			printf("New contents: %s\n", buf);
		} else {
			//_IO_SET_WRITE_NBYTES(ctp, 0);
			printf("Queue is full, sorry m8\n");
		}
		_IO_SET_WRITE_NBYTES(ctp, strlen(buf));
		// Check if we have clients waiting
		respond_to_blocked_client(ctp, fifo_rem_blocked_id(&queue));

		pthread_mutex_unlock(&queue.resource_mutex);

		// increase the offset (new reads will not get the same data)
		ocb->offset = 1;
		// return
		return (_RESMGR_NPARTS(0));
	}
	else
	{
		// set to return no data
		_IO_SET_WRITE_NBYTES(ctp, 0);

		// return
		return (_RESMGR_NPARTS(0));
	}
}

/*
 * !!NOTE: this should be called only by the thread holding the mutex, otherwise
 * it may yield undefined results due to race conditions!!
 */
void respond_to_blocked_client(resmgr_context_t *ctp, int client_id) {
	if (client_id == -1)
		return;
	// Send reply to the waiting client
	fifo_rem_string(&queue, buf);
	// set data to return
	SETIOV(ctp->iov, buf, strlen(buf));
	_IO_SET_READ_NBYTES(ctp, strlen(buf));
	ctp->offset = 1;

	MsgReply(client_id, strlen(buf), &buf, strlen(buf));
}
