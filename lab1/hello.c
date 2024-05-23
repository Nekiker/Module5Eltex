#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("Eltex");
MODULE_AUTHOR("Morozov Nikita");
MODULE_DESCRIPTION("Module 5 Lab1");

static int __init hello_init(void)
{
	printk(KERN_INFO "HELLO!!\n");
	return 0;
}

static void __exit hello_cleanup(void)
{
	printk(KERN_INFO "Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
