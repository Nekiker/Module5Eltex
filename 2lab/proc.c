#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#define MSG_SIZE 10

MODULE_LICENSE("Eltex");
MODULE_AUTHOR("Morozov Nikita");
MODULE_DESCRIPTION("Module 5 Lab 2");

static int len, temp;
static char *msg;
static const char proc_name[] = "Lab2Proc";

ssize_t read_proc(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	if (count > temp)
		count = temp;

	temp = temp - count;
	copy_to_user(buf, msg, count);
	if (count == 0)
		temp = len;
	return count;
}

ssize_t write_proc(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	copy_from_user(msg, buf, count);
	len = count;
	temp = len;
	return count;
}

static const struct proc_ops proc_fops =
{
	proc_read: read_proc,
	proc_write: write_proc
};

void create_new_proc_entry(void)
{
	proc_create(proc_name, 0, NULL, &proc_fops);
	msg = kmalloc(MSG_SIZE * sizeof(char), GFP_KERNEL);
}

static int __init hello_init(void)
{
	create_new_proc_entry();
	return 0;
}

static void __exit hello_cleanup(void)
{
	remove_proc_entry(proc_name, NULL);
	kfree(msg);
}

module_init(hello_init);
module_exit(hello_cleanup);
