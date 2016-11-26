/* Kernel Programming */
#define MODULE
#define LINUX
#define __KERNEL__

#include <linux/module.h>  /* Needed by all modules */
#include <linux/kernel.h>  /* Needed for KERN_ALERT */
#include <linux/proc_fs.h>

int procfile_read(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data)
{
	if (offset > 0)
	{	
		return 0;
	}
	else
	{
		return sprintf(buffer, "Hello proc\n");
	}
}

int init_module(void)
{
	printk("<1>Hello world 1.\n");

	struct proc_dir_entry* test = create_proc_entry("myproc", 0644, NULL);
	test->read_proc = procfile_read;	
	
	// A non 0 return means init_module failed; module can't be loaded.
	return 0;
}


void cleanup_module(void)
{
	printk(KERN_ALERT "Goodbye world 1.\n");
	remove_proc_entry("myproc", NULL);
} 

MODULE_LICENSE("GPL");

